#include "ssaoPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../../math/random.hpp"

SSAOPass::~SSAOPass() = default;

void SSAOPass::configure(const RenderContext& ctx) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mSSAO = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mSSAO->withTextureFP(false, 16)
			.checkStatus();
	mSSAOBlur = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mSSAOBlur->withTextureFP(false, 16)
			.checkStatus();

	mSSAOShader = std::make_unique<Shader>("models/quad.vert", "ssao.frag");
	mSSAOShader->activate();
	mSSAOShader->setInt("gPosition", 0);
	mSSAOShader->setInt("gNormal", 1);
	mSSAOShader->setInt("texNoise", 2);
	mSSAOShader->setInt("kernelSize", ctx.ssao.kernelSize);
	mSSAOShader->setFloat("radius", ctx.ssao.radius);
	mSSAOShader->setFloat("bias", ctx.ssao.bias);
	mSSAOShader->setVec2("resolution", glm::vec2(ctx.screen.width, ctx.screen.height));

	mSSAOBlurShader = std::make_unique<Shader>("models/quad.vert", "ssaoBlur.frag");
	mSSAOBlurShader->activate();
	mSSAOBlurShader->setInt("ssaoTexture", 0);

	mKernel.resize(ctx.ssao.kernelSize);
	mKernel = math::random::generateKernel(ctx.ssao.kernelSize);

	mNoise.resize(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoise = math::random::generateNoise(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoiseTexture = texture::generate(ctx.ssao.noiseTextureSize, ctx.ssao.noiseTextureSize, mNoise.data());

	ctx.camera.ubo.self->configure(mSSAOShader->ID(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
}

void SSAOPass::execute(const RenderContext& ctx) {
	ssao(ctx);
	blur();
}

void SSAOPass::ssao(const RenderContext& ctx) const {
	//IMPORTANT: SSAO internally converts G-buffer data to view space. G-buffer remains world-space by design.
	mSSAO->bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mSSAOShader->activate();
	for (unsigned int i = 0; i < ctx.ssao.kernelSize; ++i)
		mSSAOShader->setVec3("samples[" + std::to_string(i) + "]", mKernel[i]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->textures()[0]);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->textures()[1]);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);

	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	mSSAO->unbind();
}

void SSAOPass::blur() const {
	mSSAOBlur->bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mSSAOBlurShader->activate();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mSSAO->texture());

	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	mSSAOBlur->unbind();
}
