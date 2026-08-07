#include "deferredLighting.h"
#include "../frameGraph.h"
#include "../renderer.h"
#include "../shader.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../../config/configManager.h"

DeferredLightingPass::DeferredLightingPass() = default;

DeferredLightingPass::~DeferredLightingPass() = default;

void DeferredLightingPass::configure(const RenderContext& ctx,
                                     const FrameGraph& graph,
                                     GraphicsEncoder& encoder,
                                     EventBus& eventBus) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::None,
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
			{.code = ShaderLoader::load("models/quad2.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("deferred/lighting.frag"), .stage = ShaderStageType::Fragment},
		},
		.descriptors = {
			{
				.name = "gPosition",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureSlot")
			},
			{
				.name = "gNormal",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.normal.textureSlot")
			},
			{
				.name = "gAlbedo",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.textureSlot")
			},
			{
				.name = "gORM",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.orm.textureSlot")
			},
			{
				.name = "ssao",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.textureSlot")
			},
			{
				.name = "irradianceMap",
				.type = DescriptorType::SamplerCube,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.textureSlot")
			},
			{
				.name = "prefilterMap",
				.type = DescriptorType::SamplerCube,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.textureSlot")
			},
			{
				.name = "brdfLUT",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.textureSlot")
			},
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

	const auto& gBuffer = graph.getResource("gBuffer");
	const auto gbufferTextures = std::vector{
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.textureIdx")),
		gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.orm.textureIdx"))
	};

	const auto pbrTextures = std::vector{
		ctx.pbrBuffers->irradiance->texture(),
		ctx.pbrBuffers->prefilter->texture(),
		ctx.pbrBuffers->brdfLUT->texture()
	};

	const auto shadowTextures = std::vector{
		graph.getResource("directional").texture(),
		graph.getResource("point").texture(),
		graph.getResource("spot").texture()
	};

	encoder.bindTextures(gbufferTextures, CONFIG_MANAGER.get<int32_t>("gBuffer.position.textureSlot"));
	encoder.bindTextures(pbrTextures, CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.textureSlot"));
	encoder.bindTextures(shadowTextures, CONFIG_MANAGER.get<int32_t>("shadow.texture_slot"));
	encoder.bindTexture(
		graph.getResource("ssaoBlur").texture(),
		CONFIG_MANAGER.get<int32_t>("ssao.textureSlot"));
}

void DeferredLightingPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& gBuffer = graph.getResource("gBuffer");
	const auto& sceneBuffer = graph.getResource("sceneBuffer");

	encoder.blitFramebuffer(gBuffer, sceneBuffer, BlitMask::Depth);
	encoder.bindFrameBuffer(sceneBuffer);
	encoder.setViewport({.x = 0, .y = 0, .width = sceneBuffer.width(), .height = sceneBuffer.height()});

	encoder.bindPipeline(mPipeline);
	encoder.draw(3);
}
