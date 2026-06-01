#include "deferredGeometryPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/components/bv.hpp"

DeferredGeometryPass::~DeferredGeometryPass() = default;

const FrameBuffer* DeferredGeometryPass::gBuffer() const {
	return mGBuffer.get();
}

void DeferredGeometryPass::configure(const RenderContext& ctx, EventBus& eventBus) {
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

	mShader = ctx.resourceManager->getShader("gBuffer");
	mShader->activate();
	mShader->setInt("material.texture_albedo", ctx.PBR.albedoTextureSlot);
	mShader->setInt("material.texture_normal", ctx.PBR.normalTextureSlot);
	mShader->setInt("material.texture_roughnessMetallic", ctx.PBR.roughnessMetallicTextureSlot);
	mShader->setInt("material.texture_ao", ctx.PBR.aoTextureSlot);
	mShader->setInt("material.texture_emissive", ctx.PBR.emissiveTextureSlot);
	mShader->setInt("material.texture_height", ctx.PBR.heightTextureSlot);
}

void DeferredGeometryPass::execute(const RenderContext& ctx) {
	mGBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->activate();

	for (const auto& [entityID, materialIdx, textureOffset, textureCount, meshIdx, shader]: ctx.renderQueue->deferredObjects) {
		RenderCommon::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *mShader);
		RenderCommon::setupTransform(entityID, ctx, *mShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

		RenderCommon::drawMesh(vao, vertexCount, indexCount);
	}
}
