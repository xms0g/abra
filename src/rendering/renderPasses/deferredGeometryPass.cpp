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
#include "../../ECS/entity.hpp"

DeferredGeometryPass::~DeferredGeometryPass() = default;

const FrameBuffer* DeferredGeometryPass::gBuffer() const {
	return mGBuffer.get();
}

void DeferredGeometryPass::configure(const RenderContext& ctx) {
	mGBuffer = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mGBuffer->withTextureFP(GL_RGBA)	// position
			.withTextureFP(GL_RGBA)		// normal
#ifdef HDR
			.withTextureFP(GL_RGBA)
#else
			.withTexture(GL_RGBA)		// albedo
			.withTexture(GL_RGBA)		// orm
			.withTexture(GL_RGBA)		// emissive
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

	const Material* lastMaterial = nullptr;
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->deferredObjects) {
		RenderCommon::setupTransform(*entity, *mShader);

		if (lastMaterial != material) {
			lastMaterial = material;
			RenderCommon::setupMaterial(*entity, *lastMaterial, *mShader);
			RenderCommon::bindTextures(lastMaterial->textures, *mShader);
		}

		RenderCommon::drawMesh(*mesh);
	}

	if (lastMaterial)
		RenderCommon::unbindTextures(lastMaterial->textures);
	mGBuffer->unbind();
}
