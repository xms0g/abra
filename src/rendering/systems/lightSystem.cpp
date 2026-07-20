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

void LightSystem::configure(RenderContext& ctx, EventBus& eventBus) {
	mDirLights.reserve(CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_directional"));
	mPointLights.reserve(CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_point"));
	mSpotLights.reserve(CONFIG_MANAGER_INSTANCE.get<int32_t>("light.max_spot"));

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

	mCtx->light.dirLights = &mDirLights;
	mCtx->light.pointLights = &mPointLights;
	mCtx->light.spotLights = &mSpotLights;

	mUBO = UniformBuffer(
		DYNAMIC,
		sizeof(PackedLights),
		CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"));

	updateLightUBO();
}

void LightSystem::updateLightUBO() {
	auto updateLightData = [](const auto& sourceLights, auto& destGPUData) -> uint32_t {
		uint32_t activeCount = 0;

		for (const auto& light: sourceLights) {
			if (!light) continue;

			auto& gpuLight = destGPUData[activeCount++];

			if constexpr (requires { light->direction; }) gpuLight.direction = glm::vec4(light->direction, 0.0f);
			if constexpr (requires { light->position; }) gpuLight.position = glm::vec4(light->position, 0.0f);

			gpuLight.ambient = glm::vec4(light->ambient, 0.0f);
			gpuLight.diffuse = glm::vec4(light->diffuse, 0.0f);
			gpuLight.specular = glm::vec4(light->specular, 0.0f);

			if constexpr (requires { light->constant; }) {
				gpuLight.attenuation = glm::vec4(
					light->constant,
					light->linear,
					light->quadratic,
					static_cast<float>(light->castShadow));
			}

			if constexpr (requires { light->cutOff; }) {
				gpuLight.cutOff = glm::vec4(light->cutOff, light->outerCutOff, light->intensity, 0.0f);
			} else {
				gpuLight.intensity = glm::vec4(light->intensity, 0.0f, 0.0f, 0.0f);
			}
		}

		return activeCount;
	};

	const uint32_t dirLightCount = updateLightData(mDirLights, mGPUData.dirLights);
	const uint32_t pointLightCount = updateLightData(mPointLights, mGPUData.pointLights);
	const uint32_t spotLightCount = updateLightData(mSpotLights, mGPUData.spotLights);

	mGPUData.lightCount = glm::ivec4(dirLightCount, pointLightCount, spotLightCount, 0);

	mUBO.bind();
	mUBO.setData(&mGPUData, sizeof(PackedLights), 0);
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	auto tryProcessLight = [this]<typename TLightComponent>(
		const Entity& entity,
		const GuiLightEvent& e,
		std::vector<TLightComponent*>& lightList) {
		if (const auto& light= entity.tryGetComponent<TLightComponent>()) {
			mCtx->renderData->material.colors[e.matIdx] = light->diffuse;
			lightList[e.lightIdx] = light;
		}
	};

	const auto entityIt = std::ranges::find_if(getSystemEntities(), [event](const Entity& e) {
		return e.id() == event.entityID;
	});

	tryProcessLight.operator()<DirectionalLightComponent>(*entityIt, event, mDirLights);
	tryProcessLight.operator()<PointLightComponent>(*entityIt, event, mPointLights);
	tryProcessLight.operator()<SpotLightComponent>(*entityIt, event, mSpotLights);

	updateLightUBO();
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
