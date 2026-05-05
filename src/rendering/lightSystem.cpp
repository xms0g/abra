#include "lightSystem.h"
#include "glm/gtc/type_ptr.hpp"
#include "buffers/uniformBuffer.h"
#include "renderContext/renderContext.hpp"
#include "../config/config.hpp"
#include "../ECS/registry.h"
#include "../ECS/components/directionalLight.hpp"
#include "../ECS/components/pointLight.hpp"
#include "../ECS/components/spotLight.hpp"
#include "../event/eventBus.hpp"
#include "../event/events/guiLightEvent.hpp"

struct alignas(16) PackedLights {
	DirectionalLightComponent dirLights[MAX_DIRECTIONAL_LIGHTS];
	PointLightComponent pointLights[MAX_POINT_LIGHTS];
	SpotLightComponent spotLights[MAX_SPOT_LIGHTS];
	glm::ivec4 lightCount;
};

static PackedLights lightsData;

LightSystem::LightSystem() {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
}

void LightSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
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

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(PackedLights), ctx.light.ubo.binding);
	updateLightUBO();
}

const UniformBuffer& LightSystem::ubo() const {
	return *mUBO;
}

const std::vector<PointLightComponent*>& LightSystem::pointLights() const {
	return mPointLights;
}

const std::vector<DirectionalLightComponent*>& LightSystem::dirLights() const {
	return mDirLights;
}

const std::vector<SpotLightComponent*>& LightSystem::spotLights() const {
	return mSpotLights;
}

void LightSystem::updateLightUBO() const {
	for (size_t i = 0; i < mDirLights.size(); ++i) {
		lightsData.dirLights[i] = *mDirLights[i];
	}

	for (size_t i = 0; i < mPointLights.size(); ++i) {
		lightsData.pointLights[i] = *mPointLights[i];
	}

	for (size_t i = 0; i < mSpotLights.size(); ++i) {
		lightsData.spotLights[i] = *mSpotLights[i];
	}

	lightsData.lightCount = glm::ivec4(mDirLights.size(), mPointLights.size(), mSpotLights.size(), 0);

	mUBO->bind();
	mUBO->setData(&lightsData, sizeof(PackedLights), 0);
	mUBO->unbind();
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	for (auto& entity: getSystemEntities()) {
		if (entity.id() != event.entityID)
			continue;

		if (entity.hasComponent<DirectionalLightComponent>()) {
			mDirLights.clear();
			auto& light = entity.getComponent<DirectionalLightComponent>();

			light.direction = event.direction;
			light.ambient = event.ambient;
			light.diffuse = event.diffuse;
			light.specular = event.specular;
			light.intensity = event.intensity;

			mDirLights.push_back(&light);
		} else if (entity.hasComponent<PointLightComponent>()) {
			mPointLights.clear();

			auto& light = entity.getComponent<PointLightComponent>();

			light.position = event.position;
			light.ambient = event.ambient;
			light.diffuse = event.diffuse;
			light.specular = event.specular;
			light.attenuation = event.attenuation;
			light.intensity = event.intensity;
			light.castShadow = event.castShadow;

			mPointLights.push_back(&light);
		} else if (entity.hasComponent<SpotLightComponent>()) {
			mSpotLights.clear();

			auto& light = entity.getComponent<SpotLightComponent>();

			light.position = event.position;
			light.direction = event.direction;
			light.ambient = event.ambient;
			light.diffuse = event.diffuse;
			light.specular = event.specular;
			light.attenuation = event.attenuation;
			light.cutOff = event.cutOff;
			light.intensity = event.intensity;
			light.castShadow = event.castShadow;

			mSpotLights.push_back(&light);
		}
	}

	updateLightUBO();
}
