#include "forwardBlend.h"
#include "../frameGraph.h"
#include "../graphicsEncoder.h"
#include "../descriptorSet.h"
#include "../material/pushConstants.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../../config/configManager.h"

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
			{.name = "material.texture_height", .type = DescriptorType::SampledImage, .binding = 3},
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

	PushConstantLayout pushConstantLayout = {
		.constants = {{
			{.name = "material.flags", .offset = offsetof(MaterialPushConstants, flags), .type = PushConstantType::UInt},
			{.name = "material.heightScale", .offset = offsetof(MaterialPushConstants, heightScale), .type = PushConstantType::Float},
			{.name = "material.alphaCutoff", .offset = offsetof(MaterialPushConstants, alphaCutoff), .type = PushConstantType::Float},
			{.name = "material.color", .offset = offsetof(MaterialPushConstants, color), .type = PushConstantType::Vec3}
		}},
		.count = 4
	};

	PipelineLayout layout = {
		.descriptorSets = {materialLayout, bufferLayout, passLayout},
		.pushConstants = pushConstantLayout
	};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	DescriptorSet frameSet{};
	frameSet.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot"),
				graph.getResource("directional").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1,
				graph.getResource("point").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2,
				graph.getResource("spot").texture());

	encoder.bindDescriptorSet(frameSet);
	mCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
}

void ForwardBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;

	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
		encoder.bindPipeline(mPipeline);
		encoder.bindDescriptorSet(ctx.renderData->material.descriptorSets[cmd.material.idx]);

		const MaterialPushConstants pushConstants = {
			.flags = cmd.material.flags,
			.heightScale = cmd.material.heightScale,
			.alphaCutoff = cmd.material.alphaCutoff,
			.color = cmd.material.color
		};
		encoder.pushConstants(&pushConstants);
		encoder.setCullMode(cmd.material.flags & TWOSIDED ? CullMode::None : pipelineCullMode);
		encoder.bindTransform(cmd.transform);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
