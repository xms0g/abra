#include "ssao.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/vertexBuffer.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../mesh/vertexArray.h"
#include "../context/renderContext.hpp"
#include "../renderCommand.h"
#include "../../config/configManager.h"
#include "../../math/random.h"

SSAOPass::SSAOPass() = default;

SSAOPass::~SSAOPass() = default;

void SSAOPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	createKernel();
	createNoiseTexture();

	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = false,
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Never,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo ssaoInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("models/quad.vert", "ssao.frag"),
		.samplers = {
			{.name = "gDepthMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot")},
			{.name = "gNormal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
			{.name = "texNoise", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot")},
			{.name = "kernelSize", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.kernelSize")}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("ssao.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("ssao.ubo_binding"),
			}
		}
	};

	PipelineRenderingInfo blurInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("models/quad.vert", "ssaoBlur.frag"),
		.samplers = {
			{.name = "ssaoTexture", .slot = 0}
		},
		.uniforms = {}
	};

	mSSAOPipeline = GraphicsPipeline{ssaoInfo};
	mBlurPipeline = GraphicsPipeline{blurInfo};

	mEncoder = GraphicsEncoder{graph};

	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureIdx"));
	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx"));

	mQuad = std::make_unique<Model::SingleQuad>();
}

void SSAOPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	ssao();
	blur();
}

void SSAOPass::ssao() {
	mEncoder.reset();
	mEncoder.bindFrameBuffer("ssao");
	mEncoder.clearFrameBuffer(ClearMask::Color);

	mEncoder.bindPipeline(mSSAOPipeline);
	mEncoder.draw({
		.vao = mQuad->vao(),
		.vertexCount = 6,
		.indexCount = 0
	});
}

void SSAOPass::blur() {
	mEncoder.reset();
	mEncoder.bindFrameBuffer("ssaoBlur");
	mEncoder.clearFrameBuffer(ClearMask::Color);

	mEncoder.bindPipeline(mSSAOPipeline);
	mEncoder.bindTexture("ssao", 0);
	mEncoder.draw({
		.vao = mQuad->vao(),
		.vertexCount = 6,
		.indexCount = 0
	});
}

void SSAOPass::createKernel() {
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
	data.resolution = glm::vec4(
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"), 0.0f, 0.0f);

	mUBO = UniformBuffer{DYNAMIC, sizeof(SSAOData), CONFIG_MANAGER_INSTANCE.get<uint32_t>("ssao.ubo_binding")};
	mUBO.bind();
	mUBO.setData(&data, sizeof(SSAOData), 0);
	mUBO.unbind();
}

void SSAOPass::createNoiseTexture() {
	const int32_t textureSize = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSize");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);

	noise = math::random::generateNoise(textureSize * textureSize);
	mNoiseTexture = Texture::generate(textureSize, textureSize, noise.data());

	mNoiseTexture.bind(CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot"));
}
