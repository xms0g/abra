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

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mGBuffer = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
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

	ctx.gBuffer.buffer = mGBuffer.get();
	mShader = ResourceManager::instance().get<Shader>("gBuffer");

	const std::vector<TextureBinding> textureBindings = {
		{"material.texture_albedo", ctx.PBR.albedoTextureSlot},
		{"material.texture_normal", ctx.PBR.normalTextureSlot},
		{"material.texture_roughnessMetallic", ctx.PBR.roughnessMetallicTextureSlot},
		{"material.texture_ao", ctx.PBR.aoTextureSlot},
		{"material.texture_emissive", ctx.PBR.emissiveTextureSlot},
		{"material.texture_height", ctx.PBR.heightTextureSlot}
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
