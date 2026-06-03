#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../mesh/mesh.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mShader = ctx.resourceManager->get<Shader>("deferredLighting");

	const std::vector<TextureBinding> textureBindings = {
		{"gPosition", 0},
		{"gNormal", 1},
		{"gAlbedo", 2},
		{"gORM", 3},
		{"ssao", ctx.ssao.textureSlot},
		{"irradianceMap", ctx.PBR.irradianceMap.textureSlot},
		{"prefilterMap", ctx.PBR.prefilterMap.textureSlot},
		{"brdfLUT", ctx.PBR.brdfLUT.textureSlot}
	};

	RenderCommon::bindTextures(textureBindings, mShader);
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	ctx.gBuffer.self->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.gBuffer.self->width(), ctx.gBuffer.self->height(),
	                  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
	                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	RenderCommon::bindShadowMaps(ctx);

	ctx.gBuffer.self->bindTexture(0, ctx.gBuffer.positionTextureIdx);
	ctx.gBuffer.self->bindTexture(1, ctx.gBuffer.normalTextureIdx);
	ctx.gBuffer.self->bindTexture(2, ctx.gBuffer.albedoTextureIdx);
	ctx.gBuffer.self->bindTexture(3, ctx.gBuffer.ormTextureIdx);
	ctx.ssao.buffer->bindTexture(ctx.ssao.textureSlot);
	ctx.PBR.irradianceMap.buffer->bindTexture(ctx.PBR.irradianceMap.textureSlot);
	ctx.PBR.prefilterMap.buffer->bindTexture(ctx.PBR.prefilterMap.textureSlot);
	ctx.PBR.brdfLUT.buffer->bindTexture(ctx.PBR.brdfLUT.textureSlot);

	RenderCommon::drawQuad(mQuad->vao());
}
