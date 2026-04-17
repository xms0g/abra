#include "lightSystem.h"
#include "glm/gtc/type_ptr.hpp"
#include "buffers/uniformBuffer.h"
#include "renderContext/renderContext.hpp"
#include "../config/config.hpp"
#include "../ECS/registry.h"
#include "../ECS/components/directionalLight.hpp"
#include "../ECS/components/pointLight.hpp"
#include "../ECS/components/spotLight.hpp"
#include "../ECS/components/transform.hpp"

struct alignas(16) PackedLights {
	DirectionalLightComponent dirLights[MAX_DIRECTIONAL_LIGHTS];
	PointLightComponent pointLights[MAX_POINT_LIGHTS];
	SpotLightComponent spotLights[MAX_SPOT_LIGHTS];
	glm::ivec4 lightCount;
};

static PackedLights lightsData;

LightSystem::LightSystem(const RenderContext& ctx) {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);

	mUBO = std::make_unique<UniformBuffer>(DYNAMIC, sizeof(PackedLights), ctx.light.ubo.binding);
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

void LightSystem::update(const RenderContext& ctx) {
	mDirLights.clear();
	mPointLights.clear();
	mSpotLights.clear();

	for (auto& entity: getSystemEntities()) {
		TransformComponent tc;

		if (entity.hasComponent<TransformComponent>()) {
			tc = entity.getComponent<TransformComponent>();
		}

		if (entity.hasComponent<DirectionalLightComponent>()) {
			auto& light = entity.getComponent<DirectionalLightComponent>();
			mDirLights.push_back(&light);
		} else if (entity.hasComponent<PointLightComponent>()) {
			auto& light = entity.getComponent<PointLightComponent>();
			light.position = glm::vec4(tc.position, 1.0f);
			mPointLights.push_back(&light);
		} else if (entity.hasComponent<SpotLightComponent>()) {
			auto& light = entity.getComponent<SpotLightComponent>();
			light.position = glm::vec4(tc.position, 1.0f);
			mSpotLights.push_back(&light);
		}
	}

	updateLightUBO(ctx);
}

void LightSystem::updateLightUBO(const RenderContext& ctx) const {
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
