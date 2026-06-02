#include "ssaoPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../../math/random.h"

SSAOPass::~SSAOPass() = default;

const FrameBuffer* SSAOPass::blurFBO() const {
	return mBlurFBO.get();
}

const UniformBuffer* SSAOPass::ubo() const {
	return mUBO.get();
}

void SSAOPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Models::SingleQuad>();
	mFBO = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mFBO->withTextureFP(GL_RED)
			.checkStatus();
	mBlurFBO = std::make_unique<FrameBuffer>(ctx.screen.width, ctx.screen.height);
	mBlurFBO->withTextureFP(GL_RED)
			.checkStatus();

	mShader = ctx.resourceManager->get<Shader>("ssao");
	mShader->activate();
	mShader->setInt("gDepthMap", 0);
	mShader->setInt("gNormal", 1);
	mShader->setInt("texNoise", 2);
	mShader->setInt("kernelSize", ctx.ssao.kernelSize);
	mShader->setFloat("radius", ctx.ssao.radius);
	mShader->setFloat("bias", ctx.ssao.bias);
	mShader->setFloat("intensity", ctx.ssao.intensity);
	mShader->setVec2("resolution", glm::vec2(ctx.screen.width, ctx.screen.height));

	mBlurShader = ctx.resourceManager->get<Shader>("ssaoBlur");
	mBlurShader->activate();
	mBlurShader->setInt("ssaoTexture", 0);

	mKernel.resize(ctx.ssao.kernelSize);
	mKernel = math::random::generateKernel(ctx.ssao.kernelSize);

	mNoise.resize(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoise = math::random::generateNoise(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoiseTexture = texture::generate(ctx.ssao.noiseTextureSize, ctx.ssao.noiseTextureSize, mNoise.data());

	uint32_t uboSize = ctx.ssao.kernelSize * sizeof(glm::vec4);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, uboSize, ctx.ssao.ubo.binding);
	mUBO->bind();
	mUBO->setData(mKernel.data(), uboSize, 0);
	mUBO->unbind();

	mUBO->configure(mShader->id(), ctx.ssao.ubo.binding, ctx.ssao.ubo.blockName);
}

void SSAOPass::execute(const RenderContext& ctx) {
	ssao(ctx);
	blur();
}

void SSAOPass::ssao(const RenderContext& ctx) const {
	mFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();

	ctx.gBuffer.self->bindTexture(0, ctx.gBuffer.depthTextureIdx); // the depth texture
	ctx.gBuffer.self->bindTexture(1, ctx.gBuffer.normalTextureIdx); // normal texture

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);

	glBindVertexArray(mQuad->vao());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SSAOPass::blur() const {
	mBlurFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mBlurShader->activate();
	mFBO->bindTexture(0);

	glBindVertexArray(mQuad->vao());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
