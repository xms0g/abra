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
	mShader = std::make_unique<Shader>("models/quad.vert", "deferred/lighting.frag");
	mShader->activate();
	mShader->setInt("gPosition", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("gAlbedo", 2);
	mShader->setInt("gORM", 3);
	mShader->setInt("ssao", ctx.ssao.textureSlot);
	mShader->setInt("shadowMap", ctx.shadow.textureSlot);
	mShader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
	mShader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);
	mShader->setInt("irradianceMap", ctx.PBR.irradianceMap.textureSlot);
	mShader->setInt("prefilterMap", ctx.PBR.prefilterMap.textureSlot);
	mShader->setInt("brdfLUT", ctx.PBR.brdfLUT.textureSlot);

	ctx.camera.ubo.self->configure(mShader->id(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
	ctx.light.ubo.self->configure(mShader->id(), ctx.light.ubo.binding, ctx.light.ubo.blockName);
	ctx.shadow.ubo.self->configure(mShader->id(), ctx.shadow.ubo.binding, ctx.shadow.ubo.blockName);
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
	ctx.PBR.irradianceMap.self->bindTexture(ctx.PBR.irradianceMap.textureSlot);
	ctx.PBR.prefilterMap.self->bindTexture(ctx.PBR.prefilterMap.textureSlot);
	ctx.PBR.brdfLUT.self->bindTexture(ctx.PBR.brdfLUT.textureSlot);

	RenderCommon::drawQuad(mQuad->vao());
}