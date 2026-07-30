#include "deferredLighting.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../models/quad.h"
#include "../context/renderContext.hpp"
#include "../../config/configManager.h"
#include "../../resource/resourceManager.h"
#include "../mesh/vertexArray.h"

DeferredLightingPass::DeferredLightingPass() = default;

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.depthTestEnable = false,
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Never,
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
			{.code = ShaderLoader::load("models/quad.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("deferred/lighting.frag"), .stage = ShaderStageType::Fragment},
		},
		.samplers = {
			{.name = "gPosition", .slot = CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureSlot")},
			{.name = "gNormal", .slot = CONFIG_MANAGER.get<int32_t>("gBuffer.normal.textureSlot")},
			{.name = "gAlbedo", .slot = CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.textureSlot")},
			{.name = "gORM", .slot = CONFIG_MANAGER.get<int32_t>("gBuffer.orm.textureSlot")},
			{.name = "ssao", .slot = CONFIG_MANAGER.get<int32_t>("ssao.textureSlot")},
			{.name = "irradianceMap", .slot = CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.textureSlot")},
			{.name = "prefilterMap", .slot = CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.textureSlot")},
			{.name = "brdfLUT", .slot = CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.textureSlot")},
			{.name = "shadowMap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot")},
			{.name = "shadowCubemap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 1},
			{.name = "persShadowMap", .slot = CONFIG_MANAGER.get<int32_t>("shadow.texture_slot") + 2}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("light.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.block_name"),
				.binding = CONFIG_MANAGER.get<uint32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};

	const auto& gBuffer = graph.getResource("gBuffer");
	const auto gbufferTextures = std::vector{
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.orm.textureIdx"))
	};

	const auto pbrTextures = std::vector{
		RESOURCE_MANAGER.get<FrameBuffer>("irradianceMap")->texture(),
		RESOURCE_MANAGER.get<FrameBuffer>("prefilterMap")->texture(),
		RESOURCE_MANAGER.get<FrameBuffer>("brdfLUT")->texture()
	};

	const auto shadowTextures = std::vector{
		graph.getResource("directional").texture(),
		graph.getResource("point").texture(),
		graph.getResource("spot").texture()
	};

	mEncoder.bindTextures(gbufferTextures, CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureSlot"));
	mEncoder.bindTextures(pbrTextures, CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.textureSlot"));
	mEncoder.bindTextures(shadowTextures, CONFIG_MANAGER.get<int32_t>("shadow.texture_slot"));
	mEncoder.bindTexture(
		graph.getResource("ssaoBlur").texture(),
		CONFIG_MANAGER.get<int32_t>("ssao.textureSlot"));
	mQuad = std::make_unique<Model::Quad>();
}

void DeferredLightingPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	const auto& gBuffer = graph.getResource("gBuffer");
	const auto& sceneBuffer = graph.getResource("sceneBuffer");

	mEncoder.blitFramebuffer(gBuffer, sceneBuffer, BlitMask::Depth);
	mEncoder.bindFrameBuffer(sceneBuffer);
	mEncoder.bindPipeline(mPipeline);
	mEncoder.bindVertexArray(mQuad->vao().id());
	mEncoder.draw(6);
}
