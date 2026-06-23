#include "ssaoPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../renderContext/renderContext.hpp"
#include "../renderCommand.h"
#include "../../config/configManager.h"
#include "../../math/random.h"

SSAOPass::~SSAOPass() = default;

void SSAOPass::configure(RenderContext& ctx, EventBus& eventBus) {
	int32_t width = cfg.get<int32_t>("window.width");
	int32_t height = cfg.get<int32_t>("window.height");

	mQuad = std::make_unique<Model::SingleQuad>();

	mFBO = std::make_unique<FrameBuffer>(width, height);
	mFBO->withTextureFP(GL_RED)
			.checkStatus();

	mBlurFBO = std::make_unique<FrameBuffer>(width, height);
	mBlurFBO->withTextureFP(GL_RED)
			.checkStatus();

	ctx.ssao.buffer = mBlurFBO.get();

	mShader = rm.get<Shader>("ssao");
	mBlurShader = rm.get<Shader>("ssaoBlur");

	const std::vector<TextureBinding> ssaoTextureBindings = {
		{"gDepthMap", cfg.get<int32_t>("gBuffer.depth.textureSlot")},
		{"gNormal", cfg.get<int32_t>("gBuffer.normal.textureSlot")},
		{"texNoise", cfg.get<int32_t>("ssao.noise.textureSlot")},
		{"kernelSize", cfg.get<int32_t>("ssao.kernelSize")}
	};

	const std::vector<TextureBinding> blurTextureBindings = {
		{"ssaoTexture", 0}
	};

	RenderCommand::setTextureUnits(ssaoTextureBindings, *mShader);
	RenderCommand::setTextureUnits(blurTextureBindings, *mBlurShader);

	ctx.gBuffer->bindTexture(cfg.get<int32_t>("gBuffer.depth.textureSlot"), cfg.get<int32_t>("gBuffer.depth.textureIdx"));
	ctx.gBuffer->bindTexture(cfg.get<int32_t>("gBuffer.normal.textureSlot"), cfg.get<int32_t>("gBuffer.normal.textureIdx"));

	int32_t textureSize = cfg.get<int32_t>("ssao.noise.textureSize");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);
	noise = math::random::generateNoise(textureSize * textureSize);

	const Texture noiseTexture = texture::generate(textureSize, textureSize, noise.data());
	noiseTexture.bind(cfg.get<int32_t>("ssao.noise.textureSlot"));

	int32_t kernelSize = cfg.get<int32_t>("ssao.kernelSize");

	std::vector<glm::vec4> kernel;
	kernel.resize(kernelSize);
	kernel = math::random::generateKernel(kernelSize);

	struct alignas(16) SSAOData {
		glm::vec4 samples[32];
		glm::vec4 rbi;
		glm::vec4 resolution;
	};

	SSAOData data{};

	for (size_t i = 0; i < kernel.size(); ++i) {
		data.samples[i] = kernel[i];
	}

	data.rbi = glm::vec4(
		cfg.get<float>("ssao.radius"),
		cfg.get<float>("ssao.bias"),
		cfg.get<float>("ssao.intensity"),
		0.0f);
	data.resolution = glm::vec4(width, height, 0.0f, 0.0f);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(SSAOData), cfg.get<uint32_t>("ssao.ubo_binding"));
	mUBO->bind();
	mUBO->setData(&data, sizeof(SSAOData), 0);
	mUBO->unbind();

	mUBO->configure(
		mShader->id(),
		cfg.get<uint32_t>("ssao.ubo_binding"),
		cfg.get<std::string>("ssao.block_name").c_str());

	ctx.ssao.ubo = mUBO.get();
}

void SSAOPass::execute(const RenderContext& ctx) {
	ssao(ctx);
	blur();
}

void SSAOPass::ssao(const RenderContext& ctx) const {
	mFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	RenderCommand::drawQuad(mQuad->vao());
}

void SSAOPass::blur() const {
	mBlurFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBlurShader->activate();
	mFBO->bindTexture(0);

	RenderCommand::drawQuad(mQuad->vao());
}