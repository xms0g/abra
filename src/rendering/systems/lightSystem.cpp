#include "lightSystem.hpp"
#include <algorithm>
#include <span>
#include "glm/gtc/type_ptr.hpp"
#include "../graphicsEncoder.hpp"
#include "../buffers/uniformBuffer.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../../ECS/registry.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"
#include "../../config/configManager.hpp"

LightSystem::LightSystem() {
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
}

void LightSystem::configure(RenderContext& ctx, GraphicsEncoder& encoder, EventBus& eventBus) {
	mDirLights.reserve(CONFIG_MANAGER.get<int32_t>("light.max_directional"));
	mPointLights.reserve(CONFIG_MANAGER.get<int32_t>("light.max_point"));
	mSpotLights.reserve(CONFIG_MANAGER.get<int32_t>("light.max_spot"));

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

	mCtx->light.dirLights = std::span(mDirLights.data(), mDirLights.size());
	mCtx->light.pointLights = std::span(mPointLights.data(), mPointLights.size());
	mCtx->light.spotLights = std::span(mSpotLights.data(), mSpotLights.size());

	mUBO = UniformBuffer{sizeof(UniformBufferObject), BufferUsage::Dynamic};

	const DescriptorSetLayout layout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("light.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("light.ubo.binding"),
			}
		}
	};

	DescriptorSet lightSet{};
	lightSet.write(CONFIG_MANAGER.get<int32_t>("light.ubo.binding"), mUBO);

	encoder.bindDescriptorSet(layout, lightSet);

	updateLightUBO();
}

void LightSystem::updateLightUBO() const {
	auto updateLightData = [](const auto& sourceLights, auto& destGPUData) -> uint32_t {
		uint32_t activeCount = 0;

		for (const auto& light: sourceLights) {
			if (!light) continue;

			auto& gpuLight = destGPUData[activeCount++];

			gpuLight.position = glm::vec4(light->position, 0.0f);
			if constexpr (requires { light->direction; })
				gpuLight.direction = glm::vec4(light->direction, 0.0f);

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

	UniformBufferObject ubo{};
	const uint32_t dirLightCount = updateLightData(mDirLights, ubo.dirLights);
	const uint32_t pointLightCount = updateLightData(mPointLights, ubo.pointLights);
	const uint32_t spotLightCount = updateLightData(mSpotLights, ubo.spotLights);

	ubo.lightCount = glm::ivec4(dirLightCount, pointLightCount, spotLightCount, 0);

	mUBO.copyToMemory(&ubo, 0, sizeof(UniformBufferObject));
}

void LightSystem::onGuiUpdate(const GuiLightEvent& event) {
	const auto entityIt = std::ranges::find_if(getSystemEntities(), [event](const Entity& e) {
		return e.id() == event.entityID;
	});

	processLight<DirectionalLightComponent>(*entityIt, event, mDirLights);
	processLight<PointLightComponent>(*entityIt, event, mPointLights);
	processLight<SpotLightComponent>(*entityIt, event, mSpotLights);

	updateLightUBO();
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
