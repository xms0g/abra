#include "forwardUnlit.hpp"
#include "../shader.hpp"
#include "../frameGraph.hpp"
#include "../descriptorSet.hpp"
#include "../graphicsEncoder.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.hpp"

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

	auto materialPushConstantsLayout = MaterialPushConstants::layout;
	materialPushConstantsLayout.baseOffset = offsetof(PushConstants, material);

	auto transformPushConstantsLayout = TransformPushConstants::layout;
	transformPushConstantsLayout.baseOffset = offsetof(PushConstants, transform);

	PipelineLayout layout = {
		.descriptorSets = {bufferLayout},
		.pushConstants = {materialPushConstantsLayout, transformPushConstantsLayout}
	};

	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mCommands = &ctx.queueRegistry->get<DrawCommand>("UnlitCommands");
}

void ForwardUnlitPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
		encoder.bindPipeline(mPipeline);

		const PushConstants pushConstants = {
			.material = {
				.flags = cmd.material.flags,
				.heightScale = cmd.material.heightScale,
				.alphaCutoff = cmd.material.alphaCutoff,
				.color = cmd.material.color
			},
			.transform = {
				.model = cmd.transform.model,
				.normal = cmd.transform.normal,
			}
		};

		encoder.pushConstants(&pushConstants);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
