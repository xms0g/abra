#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mShader = std::make_unique<Shader>("models/quad.vert", "deferred/lighting.frag");
	mShader->activate();
	mShader->setInt("gPosition", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("gAlbedoSpec", 2);
	mShader->setInt("ssao", 3);
	mShader->setInt("shadowMap", ctx.shadow.textureSlot);
	mShader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
	mShader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);

	ctx.camera.ubo.self->configure(mShader->ID(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
	ctx.light.ubo.self->configure(mShader->ID(), ctx.light.ubo.binding, ctx.light.ubo.blockName);
	ctx.shadow.ubo.self->configure(mShader->ID(), ctx.shadow.ubo.binding, ctx.shadow.ubo.blockName);
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	ctx.gBuffer->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.gBuffer->width(), ctx.gBuffer->height(),
					  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	for (int i = 0; i < ctx.gBuffer->textures().size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->textures()[i]);
	}

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, ctx.ssao.buffer->texture());

	RenderCommon::bindShadowMaps(ctx);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	ctx.sceneBuffer->unbind();
}


