#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../texture/texture.h"

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mShader = rm.get<Shader>("skybox");
	mObjects = &ctx.renderQueue->get<std::vector<RenderGroup> >("skybox");

	constexpr TextureBinding textureBindings[] = {
		{"skybox", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void SkyboxPass::execute(const RenderContext& ctx) {
	const auto& [entityID, matb] = mObjects->front();
	const auto& [materialIdx, textureOffset, textureCount, s, meshes] = matb;
	const uint32_t meshIdx = meshes.front();
	const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];

	ctx.sceneBuffer->bind();
	mShader->activate();
	mShader->setMat4("skyView", ctx.camera.skyView);

	RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *mShader);
	RenderCommand::drawSkybox(vao);
}
