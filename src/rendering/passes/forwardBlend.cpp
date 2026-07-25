#include "forwardBlend.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../../config/configManager.h"

ForwardBlendPass::ForwardBlendPass() = default;

ForwardBlendPass::~ForwardBlendPass() = default;

void ForwardBlendPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("object.vert", "blend.frag"),
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

	mCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
}

void ForwardBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer("sceneBuffer");
	mEncoder.bindPipeline(mPipeline);

	for (const auto& object: *mCommands) {
		mEncoder.bindMaterial(object.material);
		mEncoder.bindTransform(object.transform);
		mEncoder.drawIndexed(object.mesh);
	}
}

