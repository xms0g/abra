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

	std::vector<ShaderStage> stages0;
	stages0.emplace_back("debug/normal.vert", ShaderStageType::Vertex);
	stages0.emplace_back("debug/normal.frag", ShaderStageType::Fragment);
	stages0.emplace_back("debug/normal.geom", ShaderStageType::Geometry);

	PipelineRenderingInfo normalInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = std::move(stages0),
		.samplers = {},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	std::vector<ShaderStage> stages1;
	stages1.emplace_back("debug/wireframe.vert", ShaderStageType::Vertex);
	stages1.emplace_back("debug/wireframe.frag", ShaderStageType::Fragment);
	stages1.emplace_back("debug/wireframe.geom", ShaderStageType::Geometry);

	PipelineRenderingInfo wireframeInfo = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = std::move(stages1),
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
