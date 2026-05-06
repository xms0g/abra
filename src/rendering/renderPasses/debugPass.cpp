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
#include "../../event/events/guiDebugEvent.hpp"

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

	for (const auto& [entityID, model, normal, material, shader, mesh]: ctx.renderQueue->dbgObjects) {
		const uint32_t mode = ctx.renderQueue->entityDebugModes.at(entityID);

		if (mode == None)
			continue;

		const auto& dbgShader = mDebugShaders.at(mode);
		dbgShader->activate();

		RenderCommon::setupTransform(entityID, model, normal, *dbgShader);
		RenderCommon::drawMesh(*mesh);
	}
}
