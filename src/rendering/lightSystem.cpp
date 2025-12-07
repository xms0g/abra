#include "lightSystem.h"
#include "glm/gtc/type_ptr.hpp"
#include "buffers/uniformBuffer.h"
#include "renderContext/renderContext.hpp"
#include "../ECS/registry.h"
#include "../ECS/components/directionalLight.hpp"
#include "../ECS/components/pointLight.hpp"
#include "../ECS/components/spotLight.hpp"
#include "../ECS/components/transform.hpp"

LightSystem::LightSystem(const RenderContext& ctx) {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);

	uint32_t totalLightBufferSize = ctx.light.maxDirLights * sizeof(DirectionalLightComponent) +
							   ctx.light.maxPointLights * sizeof(PointLightComponent) +
							   ctx.light.maxSpotLights * sizeof(SpotLightComponent) + sizeof(glm::ivec4);

	mLightUBO = std::make_unique<UniformBuffer>(totalLightBufferSize, 1);
}

void LightSystem::update(const RenderContext& ctx) {
	dirLights.clear();
	pointLights.clear();
	spotLights.clear();

	for (auto& entity: getSystemEntities()) {
		TransformComponent tc;

		if (entity.hasComponent<TransformComponent>()) {
			tc = entity.getComponent<TransformComponent>();
		}

		if (entity.hasComponent<DirectionalLightComponent>()) {
			auto& light = entity.getComponent<DirectionalLightComponent>();
			dirLights.push_back(&light);
		} else if (entity.hasComponent<PointLightComponent>()) {
			auto& light = entity.getComponent<PointLightComponent>();
			light.position = glm::vec4(tc.position, 1.0f);
			pointLights.push_back(&light);
		} else if (entity.hasComponent<SpotLightComponent>()) {
			auto& light = entity.getComponent<SpotLightComponent>();
			light.position = glm::vec4(tc.position, 1.0f);
			spotLights.push_back(&light);
		}
	}

	updateLightUBO(ctx);
}

void LightSystem::updateLightUBO(const RenderContext& ctx) const {
	mLightUBO->bind();
	uint32_t offset = 0;
	// Directional lights
	const size_t dirCount = std::min(dirLights.size(), static_cast<size_t>( ctx.light.maxDirLights));
	for (size_t i = 0; i < dirCount; i++) {
		mLightUBO->setData(dirLights[i], sizeof(DirectionalLightComponent),
						   offset + i * sizeof(DirectionalLightComponent));
	}
	offset +=  ctx.light.maxDirLights * sizeof(DirectionalLightComponent);

	// Point lights
	const size_t pointCount = std::min(pointLights.size(), static_cast<size_t>( ctx.light.maxPointLights));
	for (size_t i = 0; i < pointCount; i++) {
		mLightUBO->setData(pointLights[i], sizeof(PointLightComponent), offset + i * sizeof(PointLightComponent));
	}

	offset +=  ctx.light.maxPointLights * sizeof(PointLightComponent);

	// Spot Lights
	const size_t spotCount = std::min(spotLights.size(), static_cast<size_t>( ctx.light.maxSpotLights));
	for (size_t i = 0; i < spotCount; i++) {
		mLightUBO->setData(spotLights[i], sizeof(SpotLightComponent), offset + i * sizeof(SpotLightComponent));
	}

	offset +=  ctx.light.maxSpotLights * sizeof(SpotLightComponent);

	auto lightCount = glm::ivec4(dirCount, pointCount, spotCount, 0);
	mLightUBO->setData(glm::value_ptr(lightCount), sizeof(glm::ivec4), offset);
	mLightUBO->unbind();
}

