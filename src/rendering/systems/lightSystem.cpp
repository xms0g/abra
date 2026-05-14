#include "lightSystem.h"
#include "glm/gtc/type_ptr.hpp"
#include "../buffers/uniformBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../../config/config.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"

struct alignas(16) DirectionalLight {
	glm::vec4 direction;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 intensity;
};

struct alignas(16) PointLight {
	glm::vec4 position;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 attenuation;
	glm::vec4 intensity;
};

struct alignas(16) SpotLight {
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 attenuation;
	glm::vec4 cutOff;
};

struct alignas(16) PackedLights {
	DirectionalLight dirLights[MAX_DIRECTIONAL_LIGHTS]{};
	PointLight pointLights[MAX_POINT_LIGHTS]{};
	SpotLight spotLights[MAX_SPOT_LIGHTS]{};
	glm::ivec4 lightCount{};
};

static PackedLights lightsData;

LightSystem::LightSystem() {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
}

void LightSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
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
		lightsData.dirLights[i].direction = glm::vec4(mDirLights[i]->direction, 0.0f);
		lightsData.dirLights[i].ambient = glm::vec4(mDirLights[i]->ambient, 0.0f);
		lightsData.dirLights[i].diffuse = glm::vec4(mDirLights[i]->diffuse, 0.0f);
		lightsData.dirLights[i].specular = glm::vec4(mDirLights[i]->specular, 0.0f);
		lightsData.dirLights[i].intensity = glm::vec4(mDirLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mPointLights.size(); ++i) {
		lightsData.pointLights[i].position = glm::vec4(mPointLights[i]->position, 0.0f);
		lightsData.pointLights[i].ambient = glm::vec4(mPointLights[i]->ambient, 0.0f);
		lightsData.pointLights[i].diffuse = glm::vec4(mPointLights[i]->diffuse, 0.0f);
		lightsData.pointLights[i].specular = glm::vec4(mPointLights[i]->specular, 0.0f);
		lightsData.pointLights[i].attenuation = glm::vec4(mPointLights[i]->attenuation, static_cast<float>(mPointLights[i]->castShadow));
		lightsData.pointLights[i].intensity = glm::vec4(mPointLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mSpotLights.size(); ++i) {
		lightsData.spotLights[i].direction = glm::vec4(mSpotLights[i]->direction, 0.0f);
		lightsData.spotLights[i].position = glm::vec4(mSpotLights[i]->position, 0.0f);
		lightsData.spotLights[i].ambient = glm::vec4(mSpotLights[i]->ambient, 0.0f);
		lightsData.spotLights[i].diffuse = glm::vec4(mSpotLights[i]->diffuse, 0.0f);
		lightsData.spotLights[i].specular = glm::vec4(mSpotLights[i]->specular, 0.0f);
		lightsData.spotLights[i].attenuation = glm::vec4(mSpotLights[i]->attenuation, static_cast<float>(mSpotLights[i]->castShadow));
		lightsData.spotLights[i].cutOff = glm::vec4(mSpotLights[i]->cutOff, mSpotLights[i]->outerCutOff, mSpotLights[i]->intensity, 0.0f);
	}

	lightsData.lightCount = glm::ivec4(mDirLights.size(), mPointLights.size(), mSpotLights.size(), 0);

	mUBO->bind();
	mUBO->setData(&lightsData, sizeof(PackedLights), 0);
	mUBO->unbind();
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	mDirLights.clear();
	mPointLights.clear();
	mSpotLights.clear();

	for (auto& entity: getSystemEntities()) {
		if (entity.id() != event.entityID)
			continue;

		if (entity.hasComponent<DirectionalLightComponent>()) {
			auto& light = entity.getComponent<DirectionalLightComponent>();

			light.direction = event.direction;
			light.ambient = event.ambient;
			light.diffuse = event.diffuse;
			light.specular = event.specular;
			light.intensity = event.intensity;

			mDirLights.push_back(&light);
		} else if (entity.hasComponent<PointLightComponent>()) {
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
			auto& light = entity.getComponent<SpotLightComponent>();

			light.position = event.position;
			light.direction = event.direction;
			light.ambient = event.ambient;
			light.diffuse = event.diffuse;
			light.specular = event.specular;
			light.attenuation = event.attenuation;
			light.cutOff = event.cutOff;
			light.outerCutOff = event.outerCutOff;
			light.intensity = event.intensity;
			light.castShadow = event.castShadow;

			mSpotLights.push_back(&light);
		}
	}

	updateLightUBO();
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
