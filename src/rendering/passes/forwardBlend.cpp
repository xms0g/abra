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

	std::vector<ShaderStage> stages;
	stages.emplace_back("object.vert", ShaderStageType::Vertex);
	stages.emplace_back("blend.frag", ShaderStageType::Fragment);

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = std::move(stages),
		.samplers = {
			{.name = "material.texture_albedo", .slot = 0},
			{.name = "material.texture_specular", .slot = 1},
			{.name = "material.texture_normal", .slot = 2},
			{.name = "material.texture_height", .slot = 3},
			{.name = "shadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot")},
			{.name = "shadowCubemap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 1},
			{.name = "persShadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 2}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("light.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("shadow.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	mEncoder.bindTexture(graph.getResource("directional").texture(0), slot);
	mEncoder.bindTexture( graph.getResource("point").texture(0), slot + 1);
	mEncoder.bindTexture(graph.getResource("spot").texture(0), slot + 2);

	mCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
}

void ForwardBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		mEncoder.bindMaterial(cmd.material);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.drawIndexed(cmd.mesh);
	}
}
