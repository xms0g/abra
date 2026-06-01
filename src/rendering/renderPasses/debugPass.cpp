#include "debugPass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../event/eventBus.hpp"

DebugPass::~DebugPass() = default;

void DebugPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	mDebugShaders = {
		nullptr,
		ctx.resourceManager->getShader("debugNormal"),
		ctx.resourceManager->getShader("debugWireframe")
	};
}

void DebugPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	for (const auto& object: ctx.renderQueue->dbgObjects) {
		const uint32_t mode = ctx.renderQueue->entity.debugModes[object.entityID];

		if (mode == None)
			continue;

		const auto& dbgShader = mDebugShaders[mode];
		dbgShader->activate();

		RenderCommon::setupTransform(object.entityID, ctx, *dbgShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[object.meshIndex];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[object.meshIndex];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[object.meshIndex];

		RenderCommon::drawMesh(vao, vertexCount, indexCount);
	}
}
