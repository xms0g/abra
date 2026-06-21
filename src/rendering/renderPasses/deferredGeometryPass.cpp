#include "deferredGeometryPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../config/configManager.h"

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mGBuffer = std::make_unique<FrameBuffer>(ConfigManager::instance().window.width, ConfigManager::instance().window.height);
	mGBuffer->withTextureFP(GL_RGBA) // position
			.withTextureFP(GL_RGBA) // normal
#ifdef HDR
			.withTextureFP(GL_RGBA)
#else
			.withTexture(GL_RGBA) // albedo
			// Emissive placed into alpha channels in position, normal, albedo
#endif
			.withTexture(GL_RGBA) // orm
			.configureAttachments()
			.withTextureDepth(GL_DEPTH_COMPONENT24, false)
			.checkStatus();

	ctx.gBuffer = mGBuffer.get();
	mShader = ResourceManager::instance().get<Shader>("gBuffer");

	const std::vector<TextureBinding> textureBindings = {
		{"material.texture_albedo", ConfigManager::instance().PBR.albedoTextureSlot},
		{"material.texture_normal", ConfigManager::instance().PBR.normalTextureSlot},
		{"material.texture_roughnessMetallic", ConfigManager::instance().PBR.roughnessMetallicTextureSlot},
		{"material.texture_ao", ConfigManager::instance().PBR.aoTextureSlot},
		{"material.texture_emissive", ConfigManager::instance().PBR.emissiveTextureSlot},
		{"material.texture_height", ConfigManager::instance().PBR.heightTextureSlot}
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void DeferredGeometryPass::execute(const RenderContext& ctx) {
	mGBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->activate();

	for (const auto& [entityID, materialIdx, textureOffset, textureCount, meshIdx, shader]: ctx.renderQueue->deferredObjects) {
		RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *mShader);
		RenderCommand::setupTransform(entityID, ctx, *mShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

		RenderCommand::drawMesh(vao, vertexCount, indexCount);
	}
}
