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
	mGBuffer = std::make_unique<FrameBuffer>(
		cfg.get<int32_t>("window.width"),
		cfg.get<int32_t>("window.height"));
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
	mShader = rm.get<Shader>("gBuffer");

	const TextureBinding textureBindings[] = {
		{"material.texture_albedo", cfg.get<int32_t>("PBR.albedo.textureSlot")},
		{"material.texture_normal", cfg.get<int32_t>("PBR.normal.textureSlot")},
		{"material.texture_roughnessMetallic", cfg.get<int32_t>("PBR.roughnessMetallic.textureSlot")},
		{"material.texture_ao", cfg.get<int32_t>("PBR.ao.textureSlot")},
		{"material.texture_emissive", cfg.get<int32_t>("PBR.emissive.textureSlot")},
		{"material.texture_height", cfg.get<int32_t>("PBR.height.textureSlot")},
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
