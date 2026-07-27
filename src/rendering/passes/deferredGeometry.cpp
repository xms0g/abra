#include "deferredGeometry.h"
#include "../shader.h"
#include "../command.hpp"
#include "../graph.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

DeferredGeometryPass::DeferredGeometryPass() = default;

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("deferred/gbuffer.vert", "deferred/gbuffer.frag"),
		.samplers = {
			{
				.name = "material.texture_albedo", .
				slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.albedo.textureSlot")
			},
			{
				.name = "material.texture_normal",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.normal.textureSlot")
			},
			{
				.name = "material.texture_roughnessMetallic",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.roughnessMetallic.textureSlot")
			},
			{
				.name = "material.texture_ao",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.ao.textureSlot")
			},
			{
				.name = "material.texture_emissive",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.emissive.textureSlot")
			},
			{
				.name = "material.texture_height",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.height.textureSlot")
			},
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	mPipeline = GraphicsPipeline(desc);
	mEncoder = GraphicsEncoder(graph);
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
}

void DeferredGeometryPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer("gBuffer");
	mEncoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);
	mEncoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		mEncoder.bindMaterial(cmd.material);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.drawIndexed(cmd.mesh);
	}
}
