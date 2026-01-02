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

const FrameBuffer* SSAOPass::blurFBO() const {
	return mBlurFBO.get();
}

const UniformBuffer* SSAOPass::ubo() const {
	return mUbo.get();
}

void SSAOPass::configure(const RenderContext& ctx) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mFBO = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mFBO->withTextureFP(16, true)
			.checkStatus();
	mBlurFBO = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mBlurFBO->withTextureFP(16, true)
			.checkStatus();

	mShader = std::make_unique<Shader>("models/quad.vert", "ssao.frag");
	mShader->activate();
	mShader->setInt("gDepthMap", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("texNoise", 2);
	mShader->setInt("kernelSize", ctx.ssao.kernelSize);
	mShader->setFloat("radius", ctx.ssao.radius);
	mShader->setFloat("bias", ctx.ssao.bias);
	mShader->setVec2("resolution", glm::vec2(ctx.screen.width, ctx.screen.height));

	mBlurShader = std::make_unique<Shader>("models/quad.vert", "ssaoBlur.frag");
	mBlurShader->activate();
	mBlurShader->setInt("ssaoTexture", 0);

	mKernel.resize(ctx.ssao.kernelSize);
	mKernel = math::random::generateKernel(ctx.ssao.kernelSize);

	mNoise.resize(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoise = math::random::generateNoise(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoiseTexture = texture::generate(ctx.ssao.noiseTextureSize, ctx.ssao.noiseTextureSize, mNoise.data());

	uint32_t uboSize = ctx.ssao.kernelSize * sizeof(glm::vec4);

	mUbo = std::make_unique<UniformBuffer>(uboSize, ctx.ssao.ubo.binding);
	mUbo->bind();
	mUbo->setData(mKernel.data(), uboSize, 0);
	mUbo->unbind();

	mUbo->configure(mShader->ID(), ctx.ssao.ubo.binding, ctx.ssao.ubo.blockName);
	ctx.camera.ubo.self->configure(mShader->ID(), ctx.camera.ubo.binding, ctx.camera.ubo.blockName);
}

void SSAOPass::execute(const RenderContext& ctx) {
	ssao(ctx);
	blur();
}

void SSAOPass::ssao(const RenderContext& ctx) const {
	mFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->textures()[3]);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->textures()[1]);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);

	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	mFBO->unbind();
}

void SSAOPass::blur() const {
	mBlurFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mBlurShader->activate();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFBO->texture());

	glBindVertexArray(mQuad->VAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	mBlurFBO->unbind();
}
