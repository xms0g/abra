#include "deferredGeometry.hpp"
#include "../command.hpp"
#include "../frameGraph.hpp"
#include "../descriptorSet.hpp"
#include "../graphicsEncoder.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.hpp"

DeferredGeometryPass::DeferredGeometryPass() = default;

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(const RenderContext& ctx,
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
			{.code = ShaderLoader::load("deferred/gbuffer.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("deferred/gbuffer.frag"), .stage = ShaderStageType::Fragment},
		},
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{
				.name = "material.texture_albedo",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.albedo.slot")
			},
			{
				.name = "material.texture_normal",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.normal.slot")
			},
			{
				.name = "material.texture_roughnessMetallic",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.roughnessMetallic.slot")
			},
			{
				.name = "material.texture_ao",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.ao.slot")
			},
			{
				.name = "material.texture_emissive",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.emissive.slot")
			},
			{
				.name = "material.texture_height",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.height.slot")
			}
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
		.descriptorSets = {materialLayout, bufferLayout},
		.pushConstants = {materialPushConstantsLayout, transformPushConstantsLayout}
	};

	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mIndexes.gBuffer = graph.getResourceID("gBuffer");
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
}

void DeferredGeometryPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	encoder.bindFrameBuffer(graph.getResource(mIndexes.gBuffer));
	encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

	encoder.bindPipeline(mPipeline);

	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;
	const auto& materialLayout = mPipeline.layout().descriptorSets[0];

	for (const auto& cmd: *mCommands) {
		encoder.bindDescriptorSet(materialLayout, ctx.renderData->material.descriptorSets[cmd.material.idx]);

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
		encoder.setCullMode(cmd.material.flags & TWOSIDED ? CullMode::None : pipelineCullMode);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
