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
		nullptr, // for None
		std::make_shared<Shader>("debug/normal.vert", "debug/normal.frag", "debug/normal.geom"),
		std::make_shared<Shader>("debug/wireframe.vert", "debug/wireframe.frag", "debug/wireframe.geom")
	};

	for (const auto& shader: mDebugShaders) {
		if (!shader)
			continue;
		ctx.camera.ubo.self->configure(shader->id(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
	}
}

void DebugPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	for (const auto& [entityID, matIdx, meshIdx, shader]: ctx.renderQueue->dbgObjects) {
		const uint32_t mode = ctx.renderQueue->entity.debugModes[entityID];

		if (mode == None)
			continue;

		const auto& dbgShader = mDebugShaders[mode];
		dbgShader->activate();

		RenderCommon::setupTransform(entityID, ctx, *dbgShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

		RenderCommon::drawMesh(vao, vertexCount, indexCount);
	}
}
