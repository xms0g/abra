#include "deferredLighting.h"
#include "../graph.h"
#include "../shader.h"
#include "../models/quad.h"
#include "../context/renderContext.hpp"
#include "../../config/configManager.h"

DeferredLightingPass::DeferredLightingPass() = default;

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = false,
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Never,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("models/quad.vert", "deferred/lighting.frag"),
		.samples = {
			{.name = "gPosition", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot")},
			{.name = "gNormal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot")},
			{.name = "gAlbedo", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot")},
			{.name = "gORM", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot")},
			{.name = "ssao", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot")},
			{.name = "irradianceMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot")},
			{.name = "prefilterMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot")},
			{.name = "brdfLUT", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot")},
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

	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.position.textureIdx"));
	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.normal.textureIdx"));
	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.albedo.textureIdx"));
	mEncoder.bindTexture(
		"gBuffer",
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureSlot"),
		CONFIG_MANAGER_INSTANCE.get<int32_t>("gBuffer.orm.textureIdx"));
	mEncoder.bindTexture("ssaoBlur", CONFIG_MANAGER_INSTANCE.get<int32_t>("ssao.textureSlot"));
	mEncoder.bindTexture("irradianceMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.irradianceMap.textureSlot"));
	mEncoder.bindTexture("prefilterMap", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.prefilterMap.textureSlot"));
	mEncoder.bindTexture("brdfLUT", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.brdfLUT.textureSlot"));

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	mEncoder.bindTexture("directional", slot);
	mEncoder.bindTexture("point", slot + 1);
	mEncoder.bindTexture("spot", slot + 2);

	mQuad = std::make_unique<Model::SingleQuad>();
}

void DeferredLightingPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.blitFramebuffer("gBuffer", "sceneBuffer", BlitMask::Depth);
	mEncoder.bindFrameBuffer("sceneBuffer");
	mEncoder.bindPipeline(mPipeline);
	mEncoder.draw({
		.vao = mQuad->vao(),
		.vertexCount = 6,
		.indexCount = 0
	});
}
