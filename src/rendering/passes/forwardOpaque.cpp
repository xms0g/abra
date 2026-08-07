#include "forwardOpaque.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"
#include "../../core/gui/ui.h"

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
		},
		.descriptors = {
			{.name = "material.texture_albedo", .type = DescriptorType::Sampler2D, .binding = 0},
			{.name = "material.texture_specular", .type = DescriptorType::Sampler2D, .binding = 1},
			{.name = "material.texture_normal", .type = DescriptorType::Sampler2D, .binding = 2},
			{.name = "material.texture_height", .type = DescriptorType::Sampler2D, .binding = 3},
			{
				.name = "shadowMap",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot")
			},
			{
				.name = "shadowCubemap",
				.type = DescriptorType::SamplerCubeArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 1
			},
			{
				.name = "persShadowMap",
				.type = DescriptorType::Sampler2DArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 2
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.block_name"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("light.block_name"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.block_name"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};

	const auto shadowTextures = std::vector{
		graph.getResource("directional").texture(),
		graph.getResource("point").texture(),
		graph.getResource("spot").texture()
	};

	encoder.bindTextures(shadowTextures, CONFIG_MANAGER.get<int32_t>("shadow.texture_slot"));

	mCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
}

void ForwardOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	for (const auto& cmd: *mCommands) {
		encoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
		encoder.bindPipeline(mPipeline);
		encoder.bindMaterial(cmd.material);
		encoder.bindTransform(cmd.transform);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
