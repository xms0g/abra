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

static PackedLights gpuData;

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
			mDirLights[0] = &light;
		} else if (entity.hasComponent<PointLightComponent>()) {
			auto& light = entity.getComponent<PointLightComponent>();
			mPointLights[light.idx] = &light;
		} else if (entity.hasComponent<SpotLightComponent>()) {
			auto& light = entity.getComponent<SpotLightComponent>();
			mSpotLights[light.idx] = &light;
		}
	}

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(PackedLights), ctx.light.ubo.binding);
	updateLightUBO();
}

const UniformBuffer& LightSystem::ubo() const {
	return *mUBO;
}

std::array<DirectionalLightComponent*, MAX_DIRECTIONAL_LIGHTS>& LightSystem::dirLights() {
	return mDirLights;
}

std::array<PointLightComponent*, MAX_POINT_LIGHTS>& LightSystem::pointLights() {
	return mPointLights;
}

std::array<SpotLightComponent*, MAX_SPOT_LIGHTS>& LightSystem::spotLights() {
	return mSpotLights;
}

void LightSystem::updateLightUBO() const {
	uint32_t dirLightCount{0}, pointLightCount{0}, spotLightCount{0};

	for (size_t i = 0; i < mDirLights.size(); ++i) {
		if (!mDirLights[i]) continue;

		++dirLightCount;
		gpuData.dirLights[i].direction = glm::vec4(mDirLights[i]->direction, 0.0f);
		gpuData.dirLights[i].ambient = glm::vec4(mDirLights[i]->ambient, 0.0f);
		gpuData.dirLights[i].diffuse = glm::vec4(mDirLights[i]->diffuse, 0.0f);
		gpuData.dirLights[i].specular = glm::vec4(mDirLights[i]->specular, 0.0f);
		gpuData.dirLights[i].intensity = glm::vec4(mDirLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mPointLights.size(); ++i) {
		if (!mPointLights[i]) continue;

		++pointLightCount;
		gpuData.pointLights[i].position = glm::vec4(mPointLights[i]->position, 0.0f);
		gpuData.pointLights[i].ambient = glm::vec4(mPointLights[i]->ambient, 0.0f);
		gpuData.pointLights[i].diffuse = glm::vec4(mPointLights[i]->diffuse, 0.0f);
		gpuData.pointLights[i].specular = glm::vec4(mPointLights[i]->specular, 0.0f);
		gpuData.pointLights[i].attenuation = glm::vec4(
			mPointLights[i]->constant,
			mPointLights[i]->linear,
			mPointLights[i]->quadratic,
			static_cast<float>(mPointLights[i]->castShadow));
		gpuData.pointLights[i].intensity = glm::vec4(mPointLights[i]->intensity, 0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mSpotLights.size(); ++i) {
		if (!mSpotLights[i]) continue;

		++spotLightCount;
		gpuData.spotLights[i].direction = glm::vec4(mSpotLights[i]->direction, 0.0f);
		gpuData.spotLights[i].position = glm::vec4(mSpotLights[i]->position, 0.0f);
		gpuData.spotLights[i].ambient = glm::vec4(mSpotLights[i]->ambient, 0.0f);
		gpuData.spotLights[i].diffuse = glm::vec4(mSpotLights[i]->diffuse, 0.0f);
		gpuData.spotLights[i].specular = glm::vec4(mSpotLights[i]->specular, 0.0f);
		gpuData.spotLights[i].attenuation = glm::vec4(
			mSpotLights[i]->constant,
			mSpotLights[i]->linear,
			mSpotLights[i]->quadratic,
			static_cast<float>(mSpotLights[i]->castShadow));
		gpuData.spotLights[i].cutOff = glm::vec4(
			mSpotLights[i]->cutOff,
			mSpotLights[i]->outerCutOff,
			mSpotLights[i]->intensity,
			0.0f);
	}

	gpuData.lightCount = glm::ivec4(dirLightCount, pointLightCount, spotLightCount, 0);

	mUBO->bind();
	mUBO->setData(&gpuData, sizeof(PackedLights), 0);
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	for (auto& entity: getSystemEntities()) {
		if (entity.id() == event.entityID) {
			if (entity.hasComponent<DirectionalLightComponent>()) {
				auto& light = entity.getComponent<DirectionalLightComponent>();

				light.direction = event.direction;
				light.ambient = event.ambient;
				light.diffuse = event.diffuse;
				light.specular = event.specular;
				light.intensity = event.intensity;

				mDirLights[0] = &light;
			} else if (entity.hasComponent<PointLightComponent>()) {
				auto& light = entity.getComponent<PointLightComponent>();

				light.position = event.position;
				light.ambient = event.ambient;
				light.diffuse = event.diffuse;
				light.specular = event.specular;
				light.constant = event.constant;
				light.linear = event.linear;
				light.quadratic = event.quadratic;
				light.intensity = event.intensity;
				light.castShadow = event.castShadow;

				mPointLights[event.lightIdx] = &light;
			} else if (entity.hasComponent<SpotLightComponent>()) {
				auto& light = entity.getComponent<SpotLightComponent>();

				light.position = event.position;
				light.direction = event.direction;
				light.ambient = event.ambient;
				light.diffuse = event.diffuse;
				light.specular = event.specular;
				light.constant = event.constant;
				light.linear = event.linear;
				light.quadratic = event.quadratic;
				light.cutOff = event.cutOff;
				light.outerCutOff = event.outerCutOff;
				light.intensity = event.intensity;
				light.castShadow = event.castShadow;

				mSpotLights[event.lightIdx] = &light;
			}

			break;
		}
	}

	updateLightUBO();
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
