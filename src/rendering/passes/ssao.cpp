#include "ssao.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../descriptorSet.h"
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
	createNoiseTexture();
	createKernel(encoder);

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
		}
	};

	DescriptorSetLayout ssaoPassLayout = {
		.bindings = {
			{
				.name = "gPosition",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.position.slot")
			},
			{
				.name = "gNormal",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot")
			},
			{
				.name = "texNoise",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.noise.slot")
			}
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
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

	PipelineLayout ssaoLayout = {.descriptorSets = {ssaoPassLayout, bufferLayout}};
	GraphicsPipelineCreateInfo ssaoCreateInfo = {.rendering = ssaoInfo, .layout = ssaoLayout};
	mPipelines[0] = GraphicsPipeline{ssaoCreateInfo};

	PipelineRenderingInfo blurInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("models/quad2.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("ssaoBlur.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout blurPassLayout = {
		.bindings = {
			{.name = "ssaoTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	PipelineLayout blurLayout = {.descriptorSets = {blurPassLayout}};
	GraphicsPipelineCreateInfo blurCreateInfo = {.rendering = blurInfo, .layout = blurLayout};
	mPipelines[1] = GraphicsPipeline{blurCreateInfo};

	mIndexes.gBuffer = graph.getResourceID("gBuffer");
	mIndexes.ssao = graph.getResourceID("ssao");
	mIndexes.blur = graph.getResourceID("ssaoBlur");

	const auto& gBuffer = graph.getResource(mIndexes.gBuffer);

	DescriptorSet frameSet{};
	frameSet.write(gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.position.index")))
			.write(gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.index")))
			.write({.id = mNoiseTexture.id, .target = mNoiseTexture.target});

	encoder.bindDescriptorSet(ssaoPassLayout, frameSet);
}

void SSAOPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	ssao(graph, encoder);
	blur(graph, encoder);
}

void SSAOPass::ssao(const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& ssao = graph.getResource(mIndexes.ssao);

	encoder.bindFrameBuffer(ssao);
	encoder.clearFrameBuffer(ClearMask::Color);
	encoder.setViewport({.x = 0, .y = 0, .width = ssao.width(), .height = ssao.height()});

	encoder.bindPipeline(mPipelines[0]);
	encoder.draw(3);
}

void SSAOPass::blur(const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& blur = graph.getResource(mIndexes.blur);

	encoder.bindFrameBuffer(blur);
	encoder.clearFrameBuffer(ClearMask::Color);
	encoder.setViewport({.x = 0, .y = 0, .width = blur.width(), .height = blur.height()});

	encoder.bindPipeline(mPipelines[1]);
	encoder.bindTexture(graph.getResource(mIndexes.ssao).texture(), 0);
	encoder.draw(3);
}

void SSAOPass::createKernel(GraphicsEncoder& encoder) {
	const int32_t kernelSize = CONFIG_MANAGER.get<int32_t>("ssao.kernelSize");

	std::vector<glm::vec4> kernel;
	kernel.resize(kernelSize);
	kernel = math::random::generateKernel(kernelSize);

	struct alignas(16) UniformBufferObject {
		glm::vec4 samples[16];
		glm::vec4 settings;
		glm::vec4 resolution;
	};
	UniformBufferObject ubo{};

	for (size_t i = 0; i < kernel.size(); ++i) {
		ubo.samples[i] = kernel[i];
	}

	ubo.settings = glm::vec4(
		CONFIG_MANAGER.get<float>("ssao.radius"),
		CONFIG_MANAGER.get<float>("ssao.bias"),
		CONFIG_MANAGER.get<float>("ssao.intensity"),
		static_cast<float>(kernelSize));

	const int32_t width = CONFIG_MANAGER.get<int32_t>("window.width") / 2;
	const int32_t height = CONFIG_MANAGER.get<int32_t>("window.height") / 2;

	ubo.resolution = glm::vec4(width, height, width / 4, height / 4);

	mUBO = UniformBuffer{DYNAMIC, sizeof(UniformBufferObject)};
	const DescriptorSetLayout layout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("ssao.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.ubo.binding"),
			}
		}
	};

	DescriptorSet ssaoSet{};
	ssaoSet.write({.id = mUBO.id(), .target = mUBO.target(), .size = sizeof(UniformBufferObject)});

	encoder.bindDescriptorSet(layout, ssaoSet);
	mUBO.copyToMemory(&ubo, 0, sizeof(UniformBufferObject));
}

void SSAOPass::createNoiseTexture() {
	const int32_t textureSize = CONFIG_MANAGER.get<int32_t>("ssao.noise.size");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);

	noise = math::random::generateNoise(textureSize * textureSize);
	mNoiseTexture = Texture::generate(textureSize, textureSize, noise.data());
}
