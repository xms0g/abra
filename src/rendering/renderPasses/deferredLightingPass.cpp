#include "deferredLightingPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configureInput(const FrameBuffer* gBuffer) {
	mGBuffer = gBuffer;
}

void DeferredLightingPass::configure(const RenderContext& ctx) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mShader = std::make_unique<Shader>("models/quad.vert", "deferred/lighting.frag");
	mShader->activate();
	mShader->setInt("gPosition", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("gAlbedoSpec", 2);
	mShader->setInt("shadowMap", ctx.shadowMap.textureSlot);
	mShader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
	mShader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);

	ctx.camera.ubo->configure(mShader->ID(), 0, "CameraBlock");
	ctx.light.ubo->configure(mShader->ID(), 1, "LightBlock");
	ctx.shadowMap.ubo->configure(mShader->ID(), 2, "ShadowBlock");
}

void DeferredLightingPass::execute(const RenderContext& ctx) {
	// Copy depth buffer of gBuffer to scene buffer for the proper depth testing
	mGBuffer->bindForRead();
	ctx.sceneBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, mGBuffer->width(), mGBuffer->height(),
					  0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	ctx.sceneBuffer->bind();
	mShader->activate();

	for (int i = 0; i < mGBuffer->textures().size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mGBuffer->textures()[i]);
	}

	RenderCommon::bindShadowMaps(*ctx.shadowMap.textures);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	ctx.sceneBuffer->unbind();
}


