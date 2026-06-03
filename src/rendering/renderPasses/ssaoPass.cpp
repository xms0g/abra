#include "ssaoPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../renderCommon.h"
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
	mBlurShader = ctx.resourceManager->get<Shader>("ssaoBlur");

	const std::vector<TextureBinding> ssaoTextureBindings = {
		{"gDepthMap", 0},
		{"gNormal", 1},
		{"texNoise", 2},
		{"kernelSize", ctx.ssao.kernelSize}
	};

	const std::vector<TextureBinding> blurTextureBindings = {
		{"ssaoTexture", 0}
	};

	RenderCommon::bindTextures(ssaoTextureBindings, mShader);
	RenderCommon::bindTextures(blurTextureBindings, mBlurShader);

	std::vector<float> noise;
	noise.resize(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	noise = math::random::generateNoise(ctx.ssao.noiseTextureSize * ctx.ssao.noiseTextureSize);
	mNoiseTexture = texture::generate(ctx.ssao.noiseTextureSize, ctx.ssao.noiseTextureSize, noise.data());

	std::vector<glm::vec4> kernel;
	kernel.resize(ctx.ssao.kernelSize);
	kernel = math::random::generateKernel(ctx.ssao.kernelSize);

	struct alignas(16) SSAOData {
		glm::vec4 samples[32];
		glm::vec4 rbi;
		glm::vec4 resolution;
	};

	SSAOData data{};

	for (size_t i = 0; i < kernel.size(); ++i) {
		data.samples[i] = kernel[i];
	}

	data.rbi = glm::vec4(ctx.ssao.radius, ctx.ssao.bias, ctx.ssao.intensity, 0.0f);
	data.resolution = glm::vec4(ctx.screen.width, ctx.screen.height, 0.0f, 0.0f);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(SSAOData), ctx.ssao.ubo.binding);
	mUBO->bind();
	mUBO->setData(&data, sizeof(SSAOData), 0);
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