#include "shadowSystem.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../../frameGraph.h"
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
		},
		.descriptors = {
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

	mPipelines[0] = GraphicsPipeline{depthInfo};
	mPipelines[1] = GraphicsPipeline{cubeDepthInfo};

	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = UniformBuffer(
		DYNAMIC,
		sizeof(ShadowData),
		CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"));

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	mGPUData.omniFarPlane = glm::vec4(CONFIG_MANAGER.get<float>("shadow.omnidirectional.farPlane"), 0.0f, 0.0f,
	                                  0.0f);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass() {
	const auto lights = mCtx->light.dirLights;

	if (lights.empty())
		return;

	mDirShadow->render(*mCtx, *mGraph, *mEncoder, mPipelines[0], lights[0]->direction);
	mGPUData.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
}

void ShadowSystem::omnidirectionalShadowPass() {
	const auto lights = mCtx->light.pointLights;

	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource("point");
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

void ShadowSystem::perspectiveShadowPass() {
	const auto lights = mCtx->light.spotLights;
	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource("spot");
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

		mGPUData.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}

	mEncoder->unbindFrameBuffer();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	directionalShadowPass();
	omnidirectionalShadowPass();
	perspectiveShadowPass();

	mEncoder->setCullFace(CullMode::Back);

	mUBO.bind();
	mUBO.setData(&mGPUData, sizeof(ShadowData), 0);
}
