#include "deferredGeometry.h"
#include "../shader.h"
#include "../command.hpp"
#include "../frameGraph.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

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
		.descriptors = {
			{
				.name = "material.texture_albedo",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.albedo.slot")
			},
			{
				.name = "material.texture_normal",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.normal.slot")
			},
			{
				.name = "material.texture_roughnessMetallic",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.roughnessMetallic.slot")
			},
			{
				.name = "material.texture_ao",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.ao.slot")
			},
			{
				.name = "material.texture_emissive",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.emissive.slot")
			},
			{
				.name = "material.texture_height",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.height.slot")
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
}

void DeferredGeometryPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	encoder.bindFrameBuffer(graph.getResource("gBuffer"));
	encoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

	encoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		encoder.bindMaterial(cmd.material);
		encoder.bindTransform(cmd.transform);
		encoder.bindVertexArray(cmd.mesh.vao);
		encoder.drawIndexed(cmd.mesh.indexCount);
	}
}
