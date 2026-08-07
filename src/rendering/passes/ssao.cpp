#include "ssao.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../graphicsEncoder.h"
#include "../texture/texture.h"
#include "../context/renderContext.hpp"
#include "../../config/configManager.h"
#include "../../math/random.h"

SSAOPass::SSAOPass() = default;

SSAOPass::~SSAOPass() = default;

void SSAOPass::configure(const RenderContext& ctx,
                         const FrameGraph& graph,
                         GraphicsEncoder& encoder,
                         EventBus& eventBus) {
	createNoiseTexture(encoder);
	createKernel();

	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("models/quad2.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("ssao.frag"), .stage = ShaderStageType::Fragment}
		},
		.descriptors = {
			{
				.name = "gPosition", 
				.type = DescriptorType::Sampler2D, 
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.position.slot")
			},
			{
				.name = "gNormal", 
				.type = DescriptorType::Sampler2D, 
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot")
			},
			{
				.name = "texNoise", 
				.type = DescriptorType::Sampler2D, 
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.noise.slot")
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("ssao.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.ubo.binding"),
			}
		}
	};

	PipelineRenderingInfo blurInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("models/quad2.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("ssaoBlur.frag"), .stage = ShaderStageType::Fragment},
		},
		.descriptors = {
			{.name = "ssaoTexture", .type = DescriptorType::Sampler2D, .binding = 0}
		},
	};

	mPipelines[0] = GraphicsPipeline{ssaoInfo};
	mPipelines[1] = GraphicsPipeline{blurInfo};

	encoder = GraphicsEncoder{};

	const auto& gBuffer = graph.getResource("gBuffer");

	encoder.bindTexture(
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.position.index")),
		CONFIG_MANAGER.get<int32_t>("gBuffer.position.slot"));

	encoder.bindTexture(
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.index")),
		CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot"));
}

void SSAOPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	ssao(graph, encoder);
	blur(graph, encoder);
}

void SSAOPass::ssao(const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& ssao = graph.getResource("ssao");
	encoder.bindFrameBuffer(ssao);
	encoder.clearFrameBuffer(ClearMask::Color);
	encoder.setViewport({.x = 0, .y = 0, .width = ssao.width(), .height = ssao.height()});

	encoder.bindPipeline(mPipelines[0]);
	encoder.draw(3);
}

void SSAOPass::blur(const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& blur = graph.getResource("ssaoBlur");
	encoder.bindFrameBuffer(blur);
	encoder.clearFrameBuffer(ClearMask::Color);
	encoder.setViewport({.x = 0, .y = 0, .width = blur.width(), .height = blur.height()});

	encoder.bindPipeline(mPipelines[1]);
	encoder.bindTexture(graph.getResource("ssao").texture(), 0);
	encoder.draw(3);
}

void SSAOPass::createKernel() {
	const int32_t kernelSize = CONFIG_MANAGER.get<int32_t>("ssao.kernelSize");

	std::vector<glm::vec4> kernel;
	kernel.resize(kernelSize);
	kernel = math::random::generateKernel(kernelSize);

	struct alignas(16) SSAOData {
		glm::vec4 samples[16];
		glm::vec4 settings;
		glm::vec4 resolution;
	};
	SSAOData data{};

	for (size_t i = 0; i < kernel.size(); ++i) {
		data.samples[i] = kernel[i];
	}

	data.settings = glm::vec4(
		CONFIG_MANAGER.get<float>("ssao.radius"),
		CONFIG_MANAGER.get<float>("ssao.bias"),
		CONFIG_MANAGER.get<float>("ssao.intensity"),
		static_cast<float>(kernelSize));

	const int32_t width = CONFIG_MANAGER.get<int32_t>("window.width")/2;
	const int32_t height = CONFIG_MANAGER.get<int32_t>("window.height")/2;

	data.resolution = glm::vec4(width, height, width/4, height/4);

	mUBO = UniformBuffer{DYNAMIC, sizeof(SSAOData), CONFIG_MANAGER.get<int32_t>("ssao.ubo.binding")};
	mUBO.bind();
	mUBO.setData(&data, sizeof(SSAOData), 0);
	mUBO.unbind();
}

void SSAOPass::createNoiseTexture(GraphicsEncoder& encoder) {
	const int32_t textureSize = CONFIG_MANAGER.get<int32_t>("ssao.noise.size");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);

	noise = math::random::generateNoise(textureSize * textureSize);
	mNoiseTexture = Texture::generate(textureSize, textureSize, noise.data());

	encoder.bindTexture(
		{.id = mNoiseTexture.id, .target = mNoiseTexture.target},
		CONFIG_MANAGER.get<int32_t>("ssao.noise.slot"));
}
