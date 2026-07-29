#include "terrain.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

TerrainPass::TerrainPass() = default;

TerrainPass::~TerrainPass() = default;

void TerrainPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Patches,
	};

	constexpr PipelineTessellationState tessellationState = {
		.patchControlPoints = 4,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Line,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.tessellation = tessellationState,
		.stages = {
			{.fn = "models/terrain.vert", .type = ShaderStageType::Vertex},
			{.fn = "models/terrain.frag", .type = ShaderStageType::Fragment},
			{.fn = "models/terrain.tcs", .type = ShaderStageType::TessControl},
			{.fn = "models/terrain.tes", .type = ShaderStageType::TessEvaluation},
		},
		.samplers = {
			{.name = "material.texture_height", .slot = 0},
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};
	mCommands = &ctx.queueRegistry->get<DrawCommand>("TerrainCommands");
}

void TerrainPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& cmd = mCommands->front();

	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);
	mEncoder.bindMaterial(cmd.material);
	mEncoder.bindTransform(cmd.transform);
	mEncoder.draw(cmd.mesh);
}
