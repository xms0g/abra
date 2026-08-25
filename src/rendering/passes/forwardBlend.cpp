#include "forwardBlend.hpp"
#include "../frameGraph.hpp"
#include "../command.hpp"
#include "../graphicsEncoder.hpp"
#include "../descriptorSet.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../../config/configManager.hpp"

ForwardBlendPass::ForwardBlendPass() = default;

ForwardBlendPass::~ForwardBlendPass() = default;

void ForwardBlendPass::configure(const RenderContext& ctx,
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
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = true,
		.srcColorBlendFactor = BlendFactor::SrcAlpha,
		.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha,
		.colorBlendOp = BlendOp::Add,
		.srcAlphaBlendFactor = BlendFactor::One,
		.dstAlphaBlendFactor = BlendFactor::Zero,
		.alphaBlendOp = BlendOp::Add,
		.colorWriteMask = ColorComponent::Red | ColorComponent::Green | ColorComponent::Blue | ColorComponent::Alpha,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("object.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("blend.frag"), .stage = ShaderStageType::Fragment},
		},
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

	PushConstantLayout materialPushConstantsLayout = {
		.constants = MaterialPushConstants::layout.constants,
		.count = MaterialPushConstants::layout.count,
		.baseOffset = offsetof(PushConstants, material)
	};

	PushConstantLayout transformPushConstantsLayout = {
		.constants = TransformPushConstants::layout.constants,
		.count = TransformPushConstants::layout.count,
		.baseOffset = offsetof(PushConstants, transform)
	};

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

	mCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
}

void ForwardBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;
	const auto& materialLayout = mPipeline.layout().descriptorSets[0];

	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
		encoder.bindPipeline(mPipeline);

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
		encoder.setCullMode((cmd.material.flags & MaterialFlag::Twosided) != MaterialFlag::None ? CullMode::None : pipelineCullMode);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
