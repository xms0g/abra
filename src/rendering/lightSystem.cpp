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

	mLightUBO = std::make_unique<UniformBuffer>(totalLightBufferSize, ctx.light.uboBinding);
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

	auto uploadDataToGPU = [this, &offset]<typename T>(T& lights, const uint32_t maxLightCount) {
		using lightType = std::remove_pointer_t<typename T::value_type>;
		const size_t lightCount = std::min(lights.size(), static_cast<size_t>(maxLightCount));

		for (size_t i = 0; i < lightCount; i++) {
			mLightUBO->setData(lights[i], sizeof(lightType), offset + i * sizeof(lightType));
		}
		offset += maxLightCount * sizeof(lightType);
		return lightCount;
	};

	const size_t dirCount = uploadDataToGPU(dirLights, ctx.light.maxDirLights);
	const size_t pointCount = uploadDataToGPU(pointLights, ctx.light.maxPointLights);
	const size_t spotCount = uploadDataToGPU(spotLights, ctx.light.maxSpotLights);

	auto lightCount = glm::ivec4(dirCount, pointCount, spotCount, 0);
	mLightUBO->setData(glm::value_ptr(lightCount), sizeof(glm::ivec4), offset);
	mLightUBO->unbind();
}
