#include "forwardBlend.h"
#include "../frameGraph.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

ForwardBlendPass::ForwardBlendPass() = default;

ForwardBlendPass::~ForwardBlendPass() = default;

void ForwardBlendPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.depthTestEnable = true,
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = true,
		.srcColorBlendFactor = BlendFactor::SrcAlpha,
		.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha,
		.colorBlendOp = BlendOp::Add,
		.srcAlphaBlendFactor = BlendFactor::One,
		.dstAlphaBlendFactor = BlendFactor::Zero,
		.alphaBlendOp = BlendOp::Add,
		.colorWriteMask = ColorComponent::Red | ColorComponent::Green | ColorComponent::Blue | ColorComponent::Alpha,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("object.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("blend.frag"), .stage = ShaderStageType::Fragment},
		},
		.samplers = {
			{.name = "material.texture_albedo", .slot = 0},
			{.name = "material.texture_specular", .slot = 1},
			{.name = "material.texture_normal", .slot = 2},
			{.name = "material.texture_height", .slot = 3},
			{.name = "shadowMap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot")},
			{.name = "shadowCubemap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 1},
			{.name = "persShadowMap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 2}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("light.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};

	const auto shadowTextures = std::vector{
		graph.getResource("directional").texture(),
		graph.getResource("point").texture(),
		graph.getResource("spot").texture()
	};

	mEncoder.bindTextures(shadowTextures, CONFIG_MANAGER.get<int32_t>("shadow.texture_slot"));

	mCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
}

void ForwardBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		mEncoder.bindMaterial(cmd.material);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.bindVertexArray(cmd.mesh.vao);
		mEncoder.drawIndexed(cmd.mesh.indexCount);
	}
}
