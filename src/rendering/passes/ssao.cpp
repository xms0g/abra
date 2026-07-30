#include "ssao.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../texture/texture.h"
#include "../models/quad.h"
#include "../context/renderContext.hpp"
#include "../../config/configManager.h"
#include "../../math/random.h"
#include "../mesh/vertexArray.h"

SSAOPass::SSAOPass() = default;

SSAOPass::~SSAOPass() = default;

void SSAOPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	createNoiseTexture();
	createKernel();

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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("models/quad.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("ssao.frag"), .stage = ShaderStageType::Fragment}
		},
		.samplers = {
			{.name = "gDepthMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot")},
			{.name = "gNormal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
			{.name = "texNoise", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot")},
			{.name = "kernelSize", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.kernelSize")}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("ssao.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("ssao.ubo_binding"),
			}
		}
	};

	PipelineRenderingInfo blurInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("models/quad.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("ssaoBlur.frag"), .stage = ShaderStageType::Fragment},
		},
		.samplers = {
			{.name = "ssaoTexture", .slot = 0}
		},
		.uniforms = {}
	};

	mPipelines[0] = GraphicsPipeline{ssaoInfo};
	mPipelines[1] = GraphicsPipeline{blurInfo};

	mEncoder = GraphicsEncoder{};

	const auto& gBuffer = graph.getResource("gBuffer");

	mEncoder.bindTexture(
		gBuffer.texture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureIdx")),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.depth.textureSlot"));

	mEncoder.bindTexture(
		gBuffer.texture(CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx")),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"));

	mQuad = std::make_unique<Model::Quad>();
}

void SSAOPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	ssao(graph);
	blur(graph);
}

void SSAOPass::ssao(const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("ssao"));
	mEncoder.clearFrameBuffer(ClearMask::Color);

	mEncoder.bindPipeline(mPipelines[0]);
	mEncoder.bindVertexArray(mQuad->vao().id());
	mEncoder.draw(6);
}

void SSAOPass::blur(const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("ssaoBlur"));
	mEncoder.clearFrameBuffer(ClearMask::Color);

	mEncoder.bindPipeline(mPipelines[1]);
	mEncoder.bindTexture(graph.getResource("ssao").texture(), 0);
	mEncoder.bindVertexArray(mQuad->vao().id());
	mEncoder.draw(6);
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

	mEncoder.bindTexture(
		{.id = mNoiseTexture.id, .target = mNoiseTexture.target},
		CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.noise.textureSlot"));
}
