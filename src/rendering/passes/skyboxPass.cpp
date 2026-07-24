#include "skyboxPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderQueue.hpp"
#include "../texture/texture.h"

SkyboxPass::SkyboxPass() = default;

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("skybox");
	mObjects = &ctx.queueRegistry->get<RenderGroup>("skybox");

	constexpr TextureBinding textureBindings[] = {
		{.name = "skybox", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void SkyboxPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();
	const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];

	graph.getResource("sceneBuffer").bind();
	mShader->bind();

	RenderCommand::setupMaterial(
		entityID,
		matBatch.materialIndex,
		matBatch.textureOffset,
		matBatch.textureCount,
		ctx,
		*mShader);
	
	RenderCommand::drawSkybox(vao);
}
