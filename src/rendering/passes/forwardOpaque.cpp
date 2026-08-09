#include "forwardOpaque.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../graphicsEncoder.h"
#include "../descriptorSet.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../../config/configManager.h"

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
			{.name = "material.texture_albedo", .type = DescriptorType::Sampler2D, .binding = 0},
			{.name = "material.texture_specular", .type = DescriptorType::Sampler2D, .binding = 1},
			{.name = "material.texture_normal", .type = DescriptorType::Sampler2D, .binding = 2},
			{.name = "material.texture_height", .type = DescriptorType::Sampler2D, .binding = 3},
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
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot")
			},
			{
				.name = "shadowCubemap",
				.type = DescriptorType::SamplerCubeArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1
			},
			{
				.name = "persShadowMap",
				.type = DescriptorType::Sampler2DArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2
			},
		}
	};

	PipelineLayout layout = {.descriptorSets = {materialLayout, bufferLayout, passLayout}};
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
	mCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
}

void ForwardOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;

	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
		encoder.bindPipeline(mPipeline);
		encoder.bindDescriptorSet(ctx.renderData->material.descriptorSets[cmd.material.idx]);
		encoder.pushConstants(cmd.material);
		encoder.setCullMode(cmd.material.flags & TWOSIDED ? CullMode::None : pipelineCullMode);
		encoder.bindTransform(cmd.transform);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
