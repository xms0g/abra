#include "shadowSystem.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../../frameGraph.h"
#include "../../descriptorSet.h"
#include "../../graphicsEncoder.h"
#include "../../context/renderContext.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/updateShadowMapEvent.hpp"
#include "../../../config/configManager.h"

ShadowSystem::ShadowSystem() = default;

ShadowSystem::~ShadowSystem() = default;

void ShadowSystem::configure(const RenderContext& ctx,
                             const FrameGraph& graph,
                             GraphicsEncoder& encoder,
                             EventBus& eventBus) {
	mCtx = &ctx;
	mGraph = &graph;
	mEncoder = &encoder;
	mIndexes.directional = graph.getResourceID("directional");
	mIndexes.omnidirectional = graph.getResourceID("point");
	mIndexes.perspective = graph.getResourceID("spot");

	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Front,
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

	PipelineRenderingInfo depthInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("depth/depth.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("depth/depth.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			},
		}
	};

	PipelineRenderingInfo cubeDepthInfo = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("depth/depthCubemap.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("depth/depthCubemap.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("depth/depthCubemap.geom"), .stage = ShaderStageType::Geometry}
		},
	};

	PipelineLayout pipelineLayout = {.descriptorSets = {bufferLayout}};
	GraphicsPipelineCreateInfo depthCreateInfo = {.rendering = depthInfo, .layout = pipelineLayout};
	GraphicsPipelineCreateInfo cubeDepthCreateInfo = {.rendering = cubeDepthInfo, .layout = pipelineLayout};

	mPipelines[0] = GraphicsPipeline{depthCreateInfo};
	mPipelines[1] = GraphicsPipeline{cubeDepthCreateInfo};

	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = UniformBuffer{sizeof(UniformBufferObject), BufferUsage::Dynamic};

	const DescriptorSetLayout layout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"),
			}
		}
	};

	DescriptorSet shadowSet{};
	shadowSet.write(mUBO);

	encoder.bindDescriptorSet(layout, shadowSet);

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass(UniformBufferObject& ubo) {
	const auto lights = mCtx->light.dirLights;

	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.directional);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});
	mEncoder->clearFrameBuffer(ClearMask::Depth);

	mDirShadow->render(*mCtx, *mEncoder, mPipelines[0], lights[0]->direction);
	ubo.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
}

void ShadowSystem::omnidirectionalShadowPass() {
	const auto lights = mCtx->light.pointLights;

	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.omnidirectional);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});
	mEncoder->clearFrameBuffer(ClearMask::Depth);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mOmnidirShadow->render(*mCtx, *mEncoder, mPipelines[1], light->position, i);
	}
	mEncoder->unbindFrameBuffer();
}

void ShadowSystem::perspectiveShadowPass(UniformBufferObject& ubo) {
	const auto lights = mCtx->light.spotLights;
	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource(mIndexes.perspective);
	mEncoder->bindFrameBuffer(frameBuffer);
	mEncoder->setViewport({.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()});

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mPersShadow->render(
			*mCtx,
			*mEncoder,
			mPipelines[0],
			frameBuffer,
			light->direction,
			light->position,
			light->outerCutOff,
			i);

		ubo.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}

	mEncoder->unbindFrameBuffer();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	UniformBufferObject ubo{};
	ubo.omniFarPlane = glm::vec4(CONFIG_MANAGER.get<float>("shadow.omnidirectional.farPlane"), 0.0f, 0.0f, 0.0f);

	directionalShadowPass(ubo);
	omnidirectionalShadowPass();
	perspectiveShadowPass(ubo);

	mEncoder->setCullFace(CullMode::Back);

	mUBO.copyToMemory(&ubo, 0, sizeof(UniformBufferObject));
}
