#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../texture/texture.h"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(RenderContext& ctx, EventBus& eventBus) {
	const auto& [entity, matb] = ctx.renderQueue->skybox.front();

	constexpr TextureBinding textureBindings[] = {
		{"skybox", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *matb.shader);
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& [entityID, matb] = ctx.renderQueue->skybox.front();
	const auto& [materialIdx, textureOffset, textureCount, shader, meshes] = matb;
	const uint32_t meshIdx = meshes.front();
	const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];

	ctx.sceneBuffer->bind();
	shader->activate();
	shader->setMat4("skyView", ctx.camera.skyView);

	RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *shader);
	RenderCommand::drawSkybox(vao);
}
