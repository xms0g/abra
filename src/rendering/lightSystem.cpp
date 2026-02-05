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

	const uint32_t totalLightBufferSize = ctx.light.maxDirLights * sizeof(DirectionalLightComponent) +
	                                ctx.light.maxPointLights * sizeof(PointLightComponent) +
	                                ctx.light.maxSpotLights * sizeof(SpotLightComponent) + sizeof(glm::ivec4);

	mUBO = std::make_unique<UniformBuffer>(totalLightBufferSize, ctx.light.ubo.binding);
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
	mUBO->bind();
	uint32_t offset = 0;

	auto uploadDataToGPU = [this, &offset]<typename T>(T& lights, const uint32_t maxLightCount) -> size_t {
		using lightType = std::remove_pointer_t<typename T::value_type>;
		const size_t lightCount = std::min(lights.size(), static_cast<size_t>(maxLightCount));

		for (size_t i = 0; i < lightCount; i++) {
			mUBO->setData(lights[i], sizeof(lightType), offset + i * sizeof(lightType));
		}
		offset += maxLightCount * sizeof(lightType);
		return lightCount;
	};

	const size_t dirCount = uploadDataToGPU(mDirLights, ctx.light.maxDirLights);
	const size_t pointCount = uploadDataToGPU(mPointLights, ctx.light.maxPointLights);
	const size_t spotCount = uploadDataToGPU(mSpotLights, ctx.light.maxSpotLights);

	auto lightCount = glm::ivec4(dirCount, pointCount, spotCount, 0);
	mUBO->setData(glm::value_ptr(lightCount), sizeof(glm::ivec4), offset);
	mUBO->unbind();
}
