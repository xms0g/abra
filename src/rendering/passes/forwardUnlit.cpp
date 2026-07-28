#include "forwardUnlit.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

ForwardUnlitPass::ForwardUnlitPass() = default;

ForwardUnlitPass::~ForwardUnlitPass() = default;

void ForwardUnlitPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("unlit.vert", "unlit.frag"),
		.samplers = {},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline(info);
	mEncoder = GraphicsEncoder{};

	mCommands = &ctx.queueRegistry->get<DrawCommand>("UnlitCommands");
}

void ForwardUnlitPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		mEncoder.bindMaterial(cmd.material);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.drawIndexed(cmd.mesh);
	}
}

