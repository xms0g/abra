#pragma once
#include "glm/glm.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderData.hpp"
#include "../../ECS/system.hpp"
#include "../../event/events/guiLightEvent.hpp"

struct GuiLightEvent;
class EventBus;
class UniformBuffer;
struct PointLightComponent;
struct DirectionalLightComponent;
struct SpotLightComponent;

class LightSystem final : public System {
public:
	LightSystem();

	void configure(const RenderContext& ctx, EventBus& eventBus);

	std::vector<DirectionalLightComponent*>& dirLights();

	std::vector<PointLightComponent*>& pointLights();

	std::vector<SpotLightComponent*>& spotLights();

private:
	void updateLightUBO();

	void onGuiUpdate(const GuiLightEvent& event);

	template<typename TLightComponent, typename TLightPtr>
	void tryProcessLight(const Entity& entity, const GuiLightEvent& event, std::vector<TLightPtr*>& lightList);

	EventBus* mEventBus{};
	const RenderContext* mCtx{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::vector<DirectionalLightComponent*> mDirLights{};
	std::vector<PointLightComponent*> mPointLights{};
	std::vector<SpotLightComponent*> mSpotLights{};

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
		DirectionalLight dirLights[1]{};
		PointLight pointLights[4]{};
		SpotLight spotLights[4]{};
		glm::ivec4 lightCount{};
	};

	PackedLights mGPUData;
};

template<typename TLightComponent, typename TLightPtr>
void LightSystem::tryProcessLight(const Entity& entity, const GuiLightEvent& event, std::vector<TLightPtr*>& lightList) {
	if (entity.hasComponent<TLightComponent>()) {
		auto& light = entity.getComponent<TLightComponent>();
		mCtx->renderData->material.colors[event.matIdx] = light.diffuse;
		lightList[event.lightIdx] = &light;
	}
}
