#include "debugPass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderCommand.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/entity.hpp"

DebugPass::~DebugPass() = default;

void DebugPass::configure(const RenderContext& ctx) {
	mDebugShaders = {
		nullptr, // for None
		std::make_shared<Shader>("debug/normal.vert", "debug/normal.frag", "debug/normal.geom"),
		std::make_shared<Shader>("debug/wireframe.vert", "debug/wireframe.frag", "debug/wireframe.geom")
	};

	for (const auto& shader: mDebugShaders) {
		if (!shader) continue;
		ctx.camera.ubo->configure(shader->ID(), 0, "CameraBlock");
	}
}

void DebugPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();
	const Entity* lastEntity = nullptr;
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->dbgCommands) {
		const auto& db = entity->getComponent<DebugComponent>();
		if (db.mode == None)
			continue;

		const auto& dbgShader = mDebugShaders.at(db.mode);
		dbgShader->activate();

		if (lastEntity != entity) {
			lastEntity = entity;
			RenderCommon::setupTransform(*lastEntity, *dbgShader);
		}

		RenderCommon::drawMesh(*mesh);
	}
	ctx.sceneBuffer->unbind();
}
