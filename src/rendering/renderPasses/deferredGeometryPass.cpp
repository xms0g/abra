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
			.withTexture(GL_RGBA) // orm
			// Emissive placed into alpha channels in position, normal, albedo
#endif
			.configureAttachments()
			.withTextureDepth(GL_DEPTH_COMPONENT24, false)
			.checkStatus();

	mShader = std::make_unique<Shader>("deferred/gbuffer.vert", "deferred/gbuffer.frag");
	mShader->activate();
	mShader->setInt("material.texture_albedo", ctx.PBR.albedoTextureSlot);
	mShader->setInt("material.texture_normal", ctx.PBR.normalTextureSlot);
	mShader->setInt("material.texture_roughnessMetallic", ctx.PBR.roughnessMetallicTextureSlot);
	mShader->setInt("material.texture_ao", ctx.PBR.aoTextureSlot);
	mShader->setInt("material.texture_emissive", ctx.PBR.emissiveTextureSlot);
	mShader->setInt("material.texture_height", ctx.PBR.heightTextureSlot);

	ctx.camera.ubo.self->configure(mShader->id(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
}

void DeferredGeometryPass::execute(const RenderContext& ctx) {
	mGBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->activate();

	uint32_t lastMaterial{0};

	for (const auto& [entityID, model, normal, materialIdx, meshIdx, shader]: ctx.renderQueue->deferredObjects) {
		if (lastMaterial != materialIdx) {
			lastMaterial = materialIdx;
			const float heightScale = ctx.renderQueue->entityHeightScales[entityID];
			const float alphaCutoff = ctx.renderQueue->matAlphaCutoffs[materialIdx];
			const uint32_t flags = ctx.renderQueue->matFlags[materialIdx];
			const std::vector<uint32_t>& textures = ctx.renderQueue->matTextures[materialIdx];

			RenderCommon::setupMaterial(flags, alphaCutoff, heightScale, *mShader);
			RenderCommon::bindTextures(flags, textures, *mShader);
		}

		RenderCommon::setupTransform(entityID, model, normal, *mShader);

		const uint32_t vao = ctx.renderQueue->meshVaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->meshVertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->meshIndexCounts[meshIdx];
		RenderCommon::drawMesh(vao, vertexCount, indexCount);
	}
}
