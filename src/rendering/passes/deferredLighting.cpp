#include "deferredLighting.h"
#include "../frameGraph.h"
#include "../renderer.h"
#include "../shader.h"
#include "../graphicsEncoder.h"
#include "../descriptorSet.h"
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
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			// GBuffer
			{
				.name = "gPosition",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.position.slot")
			},
			{
				.name = "gNormal",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot")
			},
			{
				.name = "gAlbedo",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.slot")
			},
			{
				.name = "gORM",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("gBuffer.orm.slot")
			},
			// SSAO
			{
				.name = "ssao",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("ssao.slot")
			},
			// IBL
			{
				.name = "irradianceMap",
				.type = DescriptorType::SamplerCube,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.slot")
			},
			{
				.name = "prefilterMap",
				.type = DescriptorType::SamplerCube,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.slot")
			},
			{
				.name = "brdfLUT",
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.slot")
			},
			// Shadows
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
			}
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

	PipelineLayout layout = {.descriptorSets = {passLayout, bufferLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	const auto& gBuffer = graph.getResource("gBuffer");

	DescriptorSet frameSet{};
	frameSet.write(
				CONFIG_MANAGER.get<int32_t>("gBuffer.position.slot"),
				gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.position.index")))
			.write(
				CONFIG_MANAGER.get<int32_t>("gBuffer.normal.slot"),
				gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.normal.index")))
			.write(
				CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.slot"),
				gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.albedo.index")))
			.write(
				CONFIG_MANAGER.get<int32_t>("gBuffer.orm.slot"),
				gBuffer.texture(CONFIG_MANAGER.get<int32_t>("gBuffer.orm.index")))
			.write(
				CONFIG_MANAGER.get<int32_t>("ssao.slot"),
				graph.getResource("ssaoBlur").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("PBR.irradianceMap.slot"),
				ctx.pbrBuffers->irradiance->texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("PBR.prefilterMap.slot"),
				ctx.pbrBuffers->prefilter->texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("PBR.brdfLUT.slot"),
				ctx.pbrBuffers->brdfLUT->texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot"),
				graph.getResource("directional").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1,
				graph.getResource("point").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2,
				graph.getResource("spot").texture());

	encoder.bindDescriptorSet(frameSet);
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
