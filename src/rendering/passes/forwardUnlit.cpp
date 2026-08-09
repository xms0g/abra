#include "forwardUnlit.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../descriptorSet.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

ForwardUnlitPass::ForwardUnlitPass() = default;

ForwardUnlitPass::~ForwardUnlitPass() = default;

void ForwardUnlitPass::configure(const RenderContext& ctx,
                                 const FrameGraph& graph,
                                 GraphicsEncoder& encoder,
                                 EventBus& eventBus) {
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
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("unlit.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("unlit.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			}
		}
	};

	PipelineLayout layout = {.descriptorSets = {bufferLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mCommands = &ctx.queueRegistry->get<DrawCommand>("UnlitCommands");
}

void ForwardUnlitPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
		encoder.bindPipeline(mPipeline);
		encoder.pushConstants(cmd.material);
		encoder.bindTransform(cmd.transform);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
