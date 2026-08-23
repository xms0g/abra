#include "forwardOpaque.hpp"
#include "../frameGraph.hpp"
#include "../graphicsEncoder.hpp"
#include "../descriptorSet.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../../config/configManager.hpp"

ForwardOpaquePass::ForwardOpaquePass() = default;

ForwardOpaquePass::~ForwardOpaquePass() = default;

void ForwardOpaquePass::configure(const RenderContext& ctx,
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
			{.code = ShaderLoader::load("object.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("opaque.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{.name = "material.texture_albedo", .type = DescriptorType::SampledImage, .binding = 0},
			{.name = "material.texture_specular", .type = DescriptorType::SampledImage, .binding = 1},
			{.name = "material.texture_normal", .type = DescriptorType::SampledImage, .binding = 2},
		}
	};
	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("light.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("light.ubo.binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"),
			}
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			{
				.name = "shadowMap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot")
			},
			{
				.name = "shadowCubemap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1
			},
			{
				.name = "persShadowMap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2
			},
		}
	};

	auto materialPushConstantsLayout = MaterialPushConstants::layout;
	materialPushConstantsLayout.baseOffset = offsetof(PushConstants, material);

	auto transformPushConstantsLayout = TransformPushConstants::layout;
	transformPushConstantsLayout.baseOffset = offsetof(PushConstants, transform);

	PipelineLayout layout = {
		.descriptorSets = {materialLayout, bufferLayout, passLayout},
		.pushConstants = {materialPushConstantsLayout, transformPushConstantsLayout}
	};

	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");

	DescriptorSet frameSet{};
	frameSet.write(CONFIG_MANAGER.get<int32_t>("shadow.map.slot"),
	               *graph.getResource(graph.getResourceID("directional")).texture())
			.write(CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1,
			       *graph.getResource(graph.getResourceID("point")).texture())
			.write(CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2,
			       *graph.getResource(graph.getResourceID("spot")).texture());

	encoder.bindDescriptorSet(passLayout, frameSet);

	mCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
}

void ForwardOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;

	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
		encoder.bindPipeline(mPipeline);

		const auto& materialLayout = mPipeline.layout().descriptorSets[0];
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
