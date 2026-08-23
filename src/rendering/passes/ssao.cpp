#include "ssao.hpp"
#include "../frameGraph.hpp"
#include "../descriptorSet.hpp"
#include "../graphicsEncoder.hpp"
#include "../texture.hpp"
#include "../context/renderContext.hpp"
#include "../../config/configManager.hpp"
#include "../../math/random.hpp"

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
				.name = "gDepth",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.depth.slot")
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

	mIndexes.ssao = graph.getResourceID("ssao");
	mIndexes.blur = graph.getResourceID("ssaoBlur");

	const auto& gBuffer = graph.getResource(graph.getResourceID("gBuffer"));

	DescriptorSet frameSet{};
	frameSet.write(CONFIG_MANAGER.get<int32_t>("gBuffer.depth.slot"),
	               *gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.depth.index")))
			.write(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot"),
			       *gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.index")))
			.write(CONFIG_MANAGER.get<int32_t>("ssao.noise.slot"), mNoiseTexture);

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

	DescriptorSet ssaoSet{};
	ssaoSet.write(0, *graph.getResource(mIndexes.ssao).texture());
	encoder.bindDescriptorSet(mPipelines[1].layout().descriptorSets[0], ssaoSet);

	encoder.draw(3);
}

void SSAOPass::createKernel(GraphicsEncoder& encoder) {
	struct alignas(16) UniformBufferObject {
		glm::vec4 samples[16];
		glm::vec4 settings;
		glm::vec4 resolution;
	};
	UniformBufferObject ubo{};

	mUBO = UniformBuffer{sizeof(UniformBufferObject), BufferUsage::Dynamic};

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
	ssaoSet.write(CONFIG_MANAGER.get<int32_t>("ssao.ubo.binding"), mUBO);

	encoder.bindDescriptorSet(layout, ssaoSet);

	const int32_t kernelSize = CONFIG_MANAGER.get<int32_t>("ssao.kernelSize");

	std::vector<glm::vec4> kernel;
	kernel.resize(kernelSize);
	kernel = math::random::generateKernel(kernelSize);

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

	mUBO.copyToMemory(&ubo, 0, sizeof(UniformBufferObject));
}

void SSAOPass::createNoiseTexture() {
	const int32_t textureSize = CONFIG_MANAGER.get<int32_t>("ssao.noise.size");

	std::vector<float> noise;
	noise.resize(textureSize * textureSize);

	noise = math::random::generateNoise(textureSize * textureSize);
	mNoiseTexture = GPUTexture({
		.target = TextureTarget::Texture2D,
		.internalFormat = InternalFormat::RGBFloat,
		.format = BaseFormat::RGB,
		.parameters = {
			.minFilter = TextureFilter::Nearest,
			.magFilter = TextureFilter::Nearest,
			.wrapS = TextureWrap::Repeat,
			.wrapT = TextureWrap::Repeat,
		},
		.dataType = DataType::Float,
		.width = textureSize,
		.height = textureSize,
		.samples = 1,
		.layers = 1,
	});

	mNoiseTexture.copyToMemory(noise.data(), 0);
}
