#include "shadowSystem.h"
#include "glad/glad.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../../renderContext/renderContext.hpp"
#include "../../renderContext/renderQueue.hpp"
#include "../../buffers/uniformBuffer.h"
#include "../../buffers/frameBuffer.h"
#include "../../../ECS/components/directionalLight.hpp"
#include "../../../ECS/components/pointLight.hpp"
#include "../../../ECS/components/spotLight.hpp"
#include "../../../config/config.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/updateShadowMapEvent.hpp"

struct alignas(16) ShadowData {
	glm::mat4 lightSpaceMatrix{};
	glm::mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS]{};
	glm::vec4 omniFarPlane{};
};

static ShadowData gpuData{};

ShadowSystem::ShadowSystem() = default;

ShadowSystem::~ShadowSystem() = default;

const UniformBuffer* ShadowSystem::ubo() const {
	return mUBO.get();
}

void ShadowSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
	mCtx = &ctx;
	mDirShadow = std::make_unique<DirectionalShadow>(ctx);
	mOmnidirShadow = std::make_unique<OmnidirectionalShadow>(ctx);
	mPersShadow = std::make_unique<PerspectiveShadow>(ctx);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(ShadowData), ctx.shadow.ubo.binding);

	ctx.renderQueue->shadowMaps = {
		mDirShadow->depthTexture(),
		mOmnidirShadow->depthTexture(),
		mPersShadow->depthTexture()
	};

	eventBus.subscribeToEvent<ShadowSystem, UpdateShadowMapEvent>(this, &ShadowSystem::onGuiUpdate);

	gpuData.omniFarPlane = glm::vec4(ctx.shadow.omnidirectional.farPlane, 0.0f, 0.0f, 0.0f);

	constexpr UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass() const {
	const auto& lights = *mCtx->light.dirLights;

	if (!lights[0])
		return;

	mDirShadow->render(*mCtx, lights[0]->direction);
	gpuData.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
}

void ShadowSystem::omnidirectionalShadowPass() const {
	const auto& lights = *mCtx->light.pointLights;

	mOmnidirShadow->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mOmnidirShadow->render(*mCtx, light->position, i);
	}
}

void ShadowSystem::perspectiveShadowPass() const {
	const auto& lights = *mCtx->light.spotLights;

	mPersShadow->depthMap().bind();

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];

		if (!light || !light->castShadow)
			continue;

		mPersShadow->render(*mCtx, light->direction, light->position, light->outerCutOff, i);
		gpuData.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	glCullFace(GL_FRONT);

	directionalShadowPass();
	omnidirectionalShadowPass();
	perspectiveShadowPass();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(mCtx->screen.width), static_cast<int32_t>(mCtx->screen.height));

	mUBO->bind();
	mUBO->setData(&gpuData, sizeof(ShadowData), 0);
}
