#include "lightSystem.h"
#include "glm/gtc/type_ptr.hpp"
#include "../buffers/uniformBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderData.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"
#include "../../config/configManager.h"

LightSystem::LightSystem() {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
}

void LightSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
	mDirLights.reserve(cfg.get<int32_t>("light.max_directional"));
	mPointLights.reserve(cfg.get<int32_t>("light.max_point"));
	mSpotLights.reserve(cfg.get<int32_t>("light.max_spot"));

	mCtx = &ctx;
	mEventBus = &eventBus;
	eventBus.subscribeToEvent<LightSystem, GuiLightEvent>(this, &LightSystem::onGuiUpdate);

	for (auto& entity: getSystemEntities()) {
		if (entity.hasComponent<DirectionalLightComponent>()) {
			auto& light = entity.getComponent<DirectionalLightComponent>();
			mDirLights.push_back(&light);
		} else if (entity.hasComponent<PointLightComponent>()) {
			auto& light = entity.getComponent<PointLightComponent>();
			mPointLights.push_back(&light);
		} else if (entity.hasComponent<SpotLightComponent>()) {
			auto& light = entity.getComponent<SpotLightComponent>();
			mSpotLights.push_back(&light);
		}
	}

	mUBO = std::make_unique<UniformBuffer>(
		DYNAMIC,
		sizeof(PackedLights),
		cfg.get<uint32_t>("light.ubo_binding"));

	updateLightUBO();
}

std::vector<DirectionalLightComponent*>& LightSystem::dirLights() {
	return mDirLights;
}

std::vector<PointLightComponent*>& LightSystem::pointLights() {
	return mPointLights;
}

std::vector<SpotLightComponent*>& LightSystem::spotLights() {
	return mSpotLights;
}

void LightSystem::updateLightUBO() {
	uint32_t dirLightCount{0}, pointLightCount{0}, spotLightCount{0};

	for (size_t i = 0; i < mDirLights.size(); ++i) {
		if (!mDirLights[i]) continue;

		++dirLightCount;
		mGPUData.dirLights[i].direction = glm::vec4(mDirLights[i]->direction, 0.0f);
		mGPUData.dirLights[i].ambient = glm::vec4(mDirLights[i]->ambient, 0.0f);
		mGPUData.dirLights[i].diffuse = glm::vec4(mDirLights[i]->diffuse, 0.0f);
		mGPUData.dirLights[i].specular = glm::vec4(mDirLights[i]->specular, 0.0f);
		mGPUData.dirLights[i].intensity = glm::vec4(mDirLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mPointLights.size(); ++i) {
		if (!mPointLights[i]) continue;

		++pointLightCount;
		mGPUData.pointLights[i].position = glm::vec4(mPointLights[i]->position, 0.0f);
		mGPUData.pointLights[i].ambient = glm::vec4(mPointLights[i]->ambient, 0.0f);
		mGPUData.pointLights[i].diffuse = glm::vec4(mPointLights[i]->diffuse, 0.0f);
		mGPUData.pointLights[i].specular = glm::vec4(mPointLights[i]->specular, 0.0f);
		mGPUData.pointLights[i].attenuation = glm::vec4(
			mPointLights[i]->constant,
			mPointLights[i]->linear,
			mPointLights[i]->quadratic,
			static_cast<float>(mPointLights[i]->castShadow));
		mGPUData.pointLights[i].intensity = glm::vec4(mPointLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mSpotLights.size(); ++i) {
		if (!mSpotLights[i]) continue;

		++spotLightCount;
		mGPUData.spotLights[i].direction = glm::vec4(mSpotLights[i]->direction, 0.0f);
		mGPUData.spotLights[i].position = glm::vec4(mSpotLights[i]->position, 0.0f);
		mGPUData.spotLights[i].ambient = glm::vec4(mSpotLights[i]->ambient, 0.0f);
		mGPUData.spotLights[i].diffuse = glm::vec4(mSpotLights[i]->diffuse, 0.0f);
		mGPUData.spotLights[i].specular = glm::vec4(mSpotLights[i]->specular, 0.0f);
		mGPUData.spotLights[i].attenuation = glm::vec4(
			mSpotLights[i]->constant,
			mSpotLights[i]->linear,
			mSpotLights[i]->quadratic,
			static_cast<float>(mSpotLights[i]->castShadow));
		mGPUData.spotLights[i].cutOff = glm::vec4(
			mSpotLights[i]->cutOff,
			mSpotLights[i]->outerCutOff,
			mSpotLights[i]->intensity,
			0.0f);
	}

	mGPUData.lightCount = glm::ivec4(dirLightCount, pointLightCount, spotLightCount, 0);

	mUBO->bind();
	mUBO->setData(&mGPUData, sizeof(PackedLights), 0);
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	const auto entityIt = std::ranges::find_if(getSystemEntities(), [event](const Entity& e) {
		return e.id() == event.entityID;
	});

	tryProcessLight<DirectionalLightComponent>(*entityIt, event, mDirLights);
	tryProcessLight<PointLightComponent>(*entityIt, event, mPointLights);
	tryProcessLight<SpotLightComponent>(*entityIt, event, mSpotLights);

	updateLightUBO();
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
