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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("debug/normal.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("debug/normal.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("debug/normal.geom"), .stage = ShaderStageType::Geometry},
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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("debug/wireframe.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("debug/wireframe.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("debug/wireframe.geom"), .stage = ShaderStageType::Geometry},
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
	const auto& frameBuffer = graph.getResource("sceneBuffer");
	mEncoder.bindFrameBuffer(frameBuffer);
	mEncoder.setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});

	for (const auto& cmd: *mCommands) {
		if (cmd.debugMode == None)
			continue;

		mEncoder.bindPipeline(mPipelines[cmd.debugMode]);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.bindVertexArray(cmd.mesh.vao);
		mEncoder.drawIndexed(cmd.mesh.indexCount);
	}
}
