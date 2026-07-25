#include "forwardOpaque.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

ForwardOpaquePass::ForwardOpaquePass() = default;

ForwardOpaquePass::~ForwardOpaquePass() = default;

void ForwardOpaquePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.stage = Shader("object.vert", "opaque.frag"),
		.samples = {
			{.name = "shadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot")},
			{.name = "shadowCubemap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 1},
			{.name = "persShadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 2}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("light.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("shadow.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline(desc);
	mEncoder = GraphicsEncoder(graph);

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	mEncoder.bindTexture("directional", slot);
	mEncoder.bindTexture("point", slot + 1);
	mEncoder.bindTexture("spot", slot + 2);

	mCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
}

void ForwardOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer("sceneBuffer");
	mEncoder.bindPipeline(mPipeline);

	for (const auto& object: *mCommands) {
		mEncoder.bindMaterial(object.material);
		mEncoder.bindTransform(object.transform);
		mEncoder.drawIndexed(object.mesh);
	}
}
