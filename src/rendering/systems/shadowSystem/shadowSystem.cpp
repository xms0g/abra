#include "shadowSystem.h"
#include "glad/glad.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../../frameGraph.h"
#include "../../context/renderContext.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../buffers/frameBuffer.h"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/updateShadowMapEvent.hpp"
#include "../../../config/configManager.h"

ShadowSystem::ShadowSystem() = default;

ShadowSystem::~ShadowSystem() = default;

void ShadowSystem::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mCtx = &ctx;
	mGraph = &graph;
	mWidth = CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width");
	mHeight = CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height");

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
		.samplers = {},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
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
		.samplers = {},
		.uniforms = {}
	};

	mPipelines[0] = GraphicsPipeline{depthInfo};
	mPipelines[1] = GraphicsPipeline{cubeDepthInfo};
	mEncoder = GraphicsEncoder{};

	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = UniformBuffer(
		DYNAMIC,
		sizeof(ShadowData),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"));

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	mGPUData.omniFarPlane = glm::vec4(CONFIG_MANAGER_INSTANCE.get<float>("shadow.omnidirectional.farPlane"), 0.0f, 0.0f,
	                                  0.0f);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass() {
	const auto& lights = *mCtx->light.dirLights;

	if (lights.empty())
		return;

	mDirShadow->render(*mCtx, *mGraph, mEncoder, mPipelines[0], lights[0]->direction);
	mGPUData.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
}

void ShadowSystem::omnidirectionalShadowPass() {
	const auto& lights = *mCtx->light.pointLights;

	if (lights.empty())
		return;

	mEncoder.bindFrameBuffer(mGraph->getResource("point"));
	mEncoder.clearFrameBuffer(ClearMask::Depth);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mOmnidirShadow->render(*mCtx, mEncoder, mPipelines[1], light->position, i);
	}
	mEncoder.unbindFrameBuffer();
}

void ShadowSystem::perspectiveShadowPass() {
	const auto& lights = *mCtx->light.spotLights;
	if (lights.empty())
		return;

	const auto& frameBuffer = mGraph->getResource("spot");
	mEncoder.bindFrameBuffer(frameBuffer);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mPersShadow->render(*mCtx, mEncoder, mPipelines[0], frameBuffer, light->direction, light->position,
		                    light->outerCutOff, i);
		mGPUData.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}

	mEncoder.unbindFrameBuffer();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	directionalShadowPass();
	omnidirectionalShadowPass();
	perspectiveShadowPass();

	mEncoder.setCullMode(CullMode::Back);
	mEncoder.setViewport(0, 0, mWidth, mHeight);

	mUBO.bind();
	mUBO.setData(&mGPUData, sizeof(ShadowData), 0);
}
