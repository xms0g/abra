#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../texture/texture.h"

SkyboxPass::SkyboxPass() = default;

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("skybox");
	mObjects = &ctx.queueRegistry->get<RenderGroup>("skybox");

	constexpr TextureBinding textureBindings[] = {
		{.name = "skybox", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void SkyboxPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();
	const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];

	graph.getResource("sceneBuffer").bind();
	mShader->activate();
	mShader->setMat4("skyView", ctx.camera.skyView);

	RenderCommand::setupMaterial(
		entityID,
		matBatch.materialIndex,
		matBatch.textureOffset,
		matBatch.textureCount,
		ctx,
		*mShader);
	
	RenderCommand::drawSkybox(vao);
}
