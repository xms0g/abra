#include "ssaoPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/vertexBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../mesh/vertexArray.h"
#include "../renderContext/renderContext.hpp"
#include "../renderCommand.h"
#include "../../config/configManager.h"
#include "../../math/random.h"

SSAOPass::SSAOPass(const RenderGraph& graph) {
	int32_t width = CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width");
	int32_t height = CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height");

	mQuad = std::make_unique<Model::SingleQuad>();

	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("ssao");
	mBlurShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("ssaoBlur");

	const TextureBinding ssaoTextureBindings[] = {
		{"gDepthMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot")},
		{"gNormal", CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
		{"texNoise", CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot")},
		{"kernelSize", CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.kernelSize")}
	};

	constexpr TextureBinding blurTextureBindings[] = {
		{"ssaoTexture", 0}
	};

	RenderCommand::setTextureUnits(ssaoTextureBindings, *mShader);
	RenderCommand::setTextureUnits(blurTextureBindings, *mBlurShader);

	const auto& gBuffer = graph.getResource("gBuffer");
	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureIdx"));
	gBuffer.bindTexture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"), CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx"));

	const int32_t textureSize = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSize");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);
	noise = math::random::generateNoise(textureSize * textureSize);

	mNoiseTexture = Texture::generate(textureSize, textureSize, noise.data());
	mNoiseTexture.bind(CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot"));

	const int32_t kernelSize = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.kernelSize");

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
		CONFIG_MANAGER_INSTANCE.get<float>("ssao.radius"),
		CONFIG_MANAGER_INSTANCE.get<float>("ssao.bias"),
		CONFIG_MANAGER_INSTANCE.get<float>("ssao.intensity"),
		0.0f);
	data.resolution = glm::vec4(width, height, 0.0f, 0.0f);

	mUBO = UniformBuffer{DYNAMIC, sizeof(SSAOData), CONFIG_MANAGER_INSTANCE.get<uint32_t>("ssao.ubo_binding")};
	mUBO.bind();
	mUBO.setData(&data, sizeof(SSAOData), 0);
	mUBO.unbind();

	mUBO.configure(
		mShader->id(),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("ssao.ubo_binding"),
		CONFIG_MANAGER_INSTANCE.get<std::string>("ssao.block_name").c_str());
}

SSAOPass::~SSAOPass() = default;

void SSAOPass::execute(const RenderContext& ctx, RenderGraph& graph) {
	ssao(graph);
	blur(graph);
}

void SSAOPass::ssao(const RenderGraph& graph) const {
	graph.getResource("ssao").bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	RenderCommand::drawQuad(mQuad->vao());
}

void SSAOPass::blur(const RenderGraph& graph) const {
	graph.getResource("ssaoBlur").bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mBlurShader->activate();
	graph.getResource("ssao").bindTexture(0);

	RenderCommand::drawQuad(mQuad->vao());
}