#include "debugPass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/entityData.hpp"
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

	eventBus.subscribeToEvent<DebugPass, GuiDebugEvent>(this, &DebugPass::onGuiUpdate);
}

void DebugPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->dbgObjects) {
		if (entity->id != mUpdatedID || mDebugMode == None)
			continue;

		const auto& dbgShader = mDebugShaders.at(mDebugMode);
		dbgShader->activate();

		RenderCommon::setupTransform(*entity, *dbgShader);
		RenderCommon::drawMesh(*mesh);
	}
}

void DebugPass::onGuiUpdate(const GuiDebugEvent& event) {
	mUpdatedID = event.entityID;
	mDebugMode = event.mode;
}
