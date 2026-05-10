#include "shadowSystem.h"
#include "glad/glad.h"
#include "directionalShadow.h"
#include "omnidirectionalShadow.h"
#include "perspectiveShadow.h"
#include "../renderContext/renderContext.hpp"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../config/config.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"
#include "../renderContext/renderQueue.hpp"

struct alignas(16) ShadowData {
	glm::mat4 lightSpaceMatrix;
	glm::mat4 persLightSpaceMatrix[MAX_SPOT_LIGHTS];
	glm::vec4 omniFarPlanes;
};
static ShadowData shadowData{};

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

	const UpdateShadowMapEvent event;
	onGuiUpdate(event);
}

void ShadowSystem::directionalShadowPass(const RenderContext& ctx) const {
	for (const auto& light: *ctx.light.dirLights) {
		mDirShadow->render(ctx, light->direction);
		shadowData.lightSpaceMatrix = mDirShadow->lightSpaceMatrix();
	}
}

void ShadowSystem::omnidirectionalShadowPass(const RenderContext& ctx) const {
	const auto& lights = *ctx.light.pointLights;
	if (lights.empty()) return;

	mOmnidirShadow->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, static_cast<int32_t>(ctx.shadow.width), static_cast<int32_t>(ctx.shadow.height));

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow)
			continue;

		mOmnidirShadow->render(ctx, light->position, i);
		shadowData.omniFarPlanes[i] = ctx.shadow.omnidirectional.farPlane;
	}

	mOmnidirShadow->depthMap().unbind();
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
}

void ShadowSystem::perspectiveShadowPass(const RenderContext& ctx) const {
	const auto& lights = *ctx.light.spotLights;
	if (lights.empty()) return;

	mPersShadow->depthMap().bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	glViewport(0, 0, static_cast<int32_t>(ctx.shadow.width), static_cast<int32_t>(ctx.shadow.height));

	for (int32_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light->castShadow)
			continue;

		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			mPersShadow->depthMap().texture(),
			0,
			i);

		mPersShadow->render(ctx, light->direction, light->position, light->cutOff.y, i);
		shadowData.persLightSpaceMatrix[i] = mPersShadow->lightSpaceMatrix(i);
	}
	glCullFace(GL_BACK);
	glViewport(0, 0, static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
	mPersShadow->depthMap().unbind();
}

void ShadowSystem::onGuiUpdate(const UpdateShadowMapEvent& event) {
	directionalShadowPass(*mCtx);
	omnidirectionalShadowPass(*mCtx);
	perspectiveShadowPass(*mCtx);

	mUBO->bind();
	mUBO->setData(&shadowData, sizeof(ShadowData), 0);
	mUBO->unbind();
}
