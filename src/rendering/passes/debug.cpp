#include "debug.hpp"
#include "../descriptorSet.hpp"
#include "../frameGraph.hpp"
#include "../graphicsEncoder.hpp"
#include "../command.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/components/debug.hpp"
#include "../../config/configManager.hpp"

DebugPass::DebugPass() = default;

DebugPass::~DebugPass() = default;

void DebugPass::configure(const RenderContext& ctx,
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

	PipelineRenderingInfo normalInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("debug/normal.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("debug/normal.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("debug/normal.geom"), .stage = ShaderStageType::Geometry},
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
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
		}
	};

	PipelineLayout layout = {.descriptorSets = {bufferLayout}};
	GraphicsPipelineCreateInfo normalCreateInfo = {.rendering = normalInfo, .layout = layout};
	GraphicsPipelineCreateInfo wireframeCreateInfo = {.rendering = wireframeInfo, .layout = layout};

	mPipelines = {
		GraphicsPipeline{},
		GraphicsPipeline{normalCreateInfo},
		GraphicsPipeline{wireframeCreateInfo},
	};

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DebugCommands");
}

void DebugPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));

	for (const auto& cmd: *mCommands) {
		if (cmd.debugMode == None)
			continue;

		encoder.bindPipeline(mPipelines[cmd.debugMode]);
		encoder.setUniform("model", cmd.transform.model);
		encoder.setUniform("normalMatrix", cmd.transform.normal);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
