#include "debugPass.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../ECS/components/debug.hpp"

DebugPass::~DebugPass() = default;

void DebugPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mDebugShaders = {
		nullptr,
		rm.get<Shader>("debugNormal"),
		rm.get<Shader>("debugWireframe")
	};

	mObjects = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleDebug");
}

void DebugPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	for (const auto& object: *mObjects) {
		const uint32_t mode = ctx.renderData->entity.debugModes[object.entityID];

		if (mode == None)
			continue;

		const auto& dbgShader = mDebugShaders[mode];
		dbgShader->activate();

		RenderCommand::setupTransform(object.entityID, ctx, *dbgShader);

		const uint32_t vao = ctx.renderData->mesh.vaos[object.meshIndex];
		const size_t vertexCount = ctx.renderData->mesh.vertexCounts[object.meshIndex];
		const size_t indexCount = ctx.renderData->mesh.indexCounts[object.meshIndex];

		RenderCommand::drawMesh(vao, vertexCount, indexCount);
	}
}
