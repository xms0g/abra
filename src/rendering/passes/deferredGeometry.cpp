#include "deferredGeometry.h"
#include "../shader.h"
#include "../command.hpp"
#include "../frameGraph.h"
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

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("deferred/gbuffer.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("deferred/gbuffer.frag"), .stage = ShaderStageType::Fragment},
		},
		.samplers = {
			{
				.name = "material.texture_albedo",
				.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.albedo.textureSlot")
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
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};
	mCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
}

void DeferredGeometryPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	const auto& frameBuffer = graph.getResource("gBuffer");
	mEncoder.bindFrameBuffer(frameBuffer);
	mEncoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);
	mEncoder.setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});

	mEncoder.bindPipeline(mPipeline);

	for (const auto& cmd: *mCommands) {
		mEncoder.bindMaterial(cmd.material);
		mEncoder.bindTransform(cmd.transform);
		mEncoder.bindVertexArray(cmd.mesh.vao);
		mEncoder.drawIndexed(cmd.mesh.indexCount);
	}
}
