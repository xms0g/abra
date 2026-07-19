#include "shadowSystem.h"
#include "glad/glad.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../../renderGraph.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderData.hpp"
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

void ShadowSystem::configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) {
	mCtx = &ctx;
	mGraph = &graph;
	mWidth= CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width");
	mHeight = CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height");

	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = std::make_unique<UniformBuffer>(
		DYNAMIC,
		sizeof(ShadowData),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"));

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	mGPUData.omniFarPlane = glm::vec4(CONFIG_MANAGER_INSTANCE.get<float>("shadow.omnidirectional.farPlane"), 0.0f, 0.0f, 0.0f);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass() {
	const auto& lights = *mCtx->light.dirLights;

	if (lights.empty())
		return;

	mDirShadow->render(*mCtx, *mGraph, lights[0]->direction);
	mGPUData.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
}

void ShadowSystem::omnidirectionalShadowPass() const {
	const auto& lights = *mCtx->light.pointLights;

	if (lights.empty())
		return;

	mGraph->getResource("point").bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mOmnidirShadow->render(*mCtx, light->position, i);
	}

	mGraph->getResource("point").unbind();
}

void ShadowSystem::perspectiveShadowPass() {
	const auto& lights = *mCtx->light.spotLights;
	if (lights.empty())
		return;

	const auto& depthBuffer = mGraph->getResource("spot");

	depthBuffer.bind();
	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mPersShadow->render(*mCtx, light->direction, light->position, light->outerCutOff, depthBuffer.texture(), i);
		mGPUData.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}
	depthBuffer.unbind();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	glCullFace(GL_FRONT);

	directionalShadowPass();
	omnidirectionalShadowPass();
	perspectiveShadowPass();

	glCullFace(GL_BACK);
	glViewport(0, 0, mWidth, mHeight);

	mUBO->bind();
	mUBO->setData(&mGPUData, sizeof(ShadowData), 0);
}
