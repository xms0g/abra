#include "debug.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../config/configManager.h"

DebugPass::DebugPass() = default;

DebugPass::~DebugPass() = default;

void DebugPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo normalInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = {
			{.fn = "debug/normal.vert", .type = ShaderStageType::Vertex},
			{.fn = "debug/normal.frag", .type = ShaderStageType::Fragment},
			{.fn = "debug/normal.geom", .type = ShaderStageType::Geometry}
		},
		.samplers = {},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	PipelineRenderingInfo wireframeInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = {
			{.fn = "debug/wireframe.vert", .type = ShaderStageType::Vertex},
			{.fn = "debug/wireframe.frag", .type = ShaderStageType::Fragment},
			{.fn = "debug/wireframe.geom", .type = ShaderStageType::Geometry},
		},
		.samplers = {},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	mPipelines = {
		GraphicsPipeline{},
		GraphicsPipeline{normalInfo},
		GraphicsPipeline{wireframeInfo},
	};

	mEncoder = GraphicsEncoder{};
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DebugCommands");
}

void DebugPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));

	for (const auto& cmd: *mCommands) {
		if (cmd.debugMode == None)
			continue;

		mEncoder.bindPipeline(mPipelines[cmd.debugMode]);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.drawIndexed(cmd.mesh);
	}
}
