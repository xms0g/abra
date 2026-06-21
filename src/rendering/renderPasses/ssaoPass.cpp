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
	mQuad = std::make_unique<Model::SingleQuad>();
	mFBO = std::make_unique<FrameBuffer>(ConfigManager::instance().window.width, ConfigManager::instance().window.height);
	mFBO->withTextureFP(GL_RED)
			.checkStatus();
	mBlurFBO = std::make_unique<FrameBuffer>(ConfigManager::instance().window.width, ConfigManager::instance().window.height);
	mBlurFBO->withTextureFP(GL_RED)
			.checkStatus();

	ctx.ssao.buffer = mBlurFBO.get();

	mShader = ResourceManager::instance().get<Shader>("ssao");
	mBlurShader = ResourceManager::instance().get<Shader>("ssaoBlur");

	const std::vector<TextureBinding> ssaoTextureBindings = {
		{"gDepthMap", ConfigManager::instance().gBuffer.depth.textureSlot},
		{"gNormal", ConfigManager::instance().gBuffer.normal.textureSlot},
		{"texNoise", ConfigManager::instance().ssao.noise.textureSlot},
		{"kernelSize", ConfigManager::instance().ssao.kernelSize}
	};

	const std::vector<TextureBinding> blurTextureBindings = {
		{"ssaoTexture", 0}
	};

	RenderCommand::setTextureUnits(ssaoTextureBindings, *mShader);
	RenderCommand::setTextureUnits(blurTextureBindings, *mBlurShader);

	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.depth.textureSlot, ConfigManager::instance().gBuffer.depth.textureIdx);
	ctx.gBuffer->bindTexture(ConfigManager::instance().gBuffer.normal.textureSlot, ConfigManager::instance().gBuffer.normal.textureIdx);

	int32_t textureSize = ConfigManager::instance().ssao.noise.textureSize;

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);
	noise = math::random::generateNoise(textureSize * textureSize);

	const Texture noiseTexture = texture::generate(textureSize, textureSize, noise.data());
	noiseTexture.bind(ConfigManager::instance().ssao.noise.textureSlot);

	std::vector<glm::vec4> kernel;
	kernel.resize(ConfigManager::instance().ssao.kernelSize);
	kernel = math::random::generateKernel(ConfigManager::instance().ssao.kernelSize);

	struct alignas(16) SSAOData {
		glm::vec4 samples[32];
		glm::vec4 rbi;
		glm::vec4 resolution;
	};

	SSAOData data{};

	for (size_t i = 0; i < kernel.size(); ++i) {
		data.samples[i] = kernel[i];
	}

	data.rbi = glm::vec4(ConfigManager::instance().ssao.radius, ConfigManager::instance().ssao.bias, ConfigManager::instance().ssao.intensity, 0.0f);
	data.resolution = glm::vec4(ConfigManager::instance().window.width, ConfigManager::instance().window.height, 0.0f, 0.0f);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(SSAOData), ConfigManager::instance().ssao.ubo_binding);
	mUBO->bind();
	mUBO->setData(&data, sizeof(SSAOData), 0);
	mUBO->unbind();

	mUBO->configure(mShader->id(), ConfigManager::instance().ssao.ubo_binding, ConfigManager::instance().ssao.block_name.c_str());
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