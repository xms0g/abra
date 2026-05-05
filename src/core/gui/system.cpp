#include "system.h"
#include "panels.h"
#include "ui.h"
#include "entityState.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/instance.hpp"
#include "../../event/eventBus.hpp"

GuiSystem::GuiSystem() {
	RequireComponent<TransformComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<DebugComponent>(true);
}

GuiSystem::~GuiSystem() = default;

void GuiSystem::update(const float dt) {
	updateFpsCounter(dt);
}

void GuiSystem::configure() {
	for (const auto& entity: getSystemEntities()) {
		const auto& transform = entity.getComponent<TransformComponent>();
		glm::vec4 direction, ambient, diffuse, specular;
		glm::vec3 attenuation, cutOff;
		float intensity{1.0f};
		bool castShadow{false};

		if (entity.hasComponent<DirectionalLightComponent>()) {
			direction = entity.getComponent<DirectionalLightComponent>().direction;
			ambient = entity.getComponent<DirectionalLightComponent>().ambient;
			diffuse = entity.getComponent<DirectionalLightComponent>().diffuse;
			specular = entity.getComponent<DirectionalLightComponent>().specular;
			intensity = entity.getComponent<DirectionalLightComponent>().intensity;
			attenuation = glm::vec3(0.0f);
			cutOff = glm::vec3(0.0f);
		} else if (entity.hasComponent<PointLightComponent>()) {
			ambient = entity.getComponent<PointLightComponent>().ambient;
			diffuse = entity.getComponent<PointLightComponent>().diffuse;
			specular = entity.getComponent<PointLightComponent>().specular;
			intensity = entity.getComponent<PointLightComponent>().intensity;
			attenuation = entity.getComponent<PointLightComponent>().attenuation;
			castShadow = entity.getComponent<PointLightComponent>().castShadow;
			cutOff = glm::vec3(0.0f);
			direction = glm::vec4(0.0f);
		} else if (entity.hasComponent<SpotLightComponent>()) {
			direction = entity.getComponent<SpotLightComponent>().direction;
			ambient = entity.getComponent<SpotLightComponent>().ambient;
			diffuse = entity.getComponent<SpotLightComponent>().diffuse;
			specular = entity.getComponent<SpotLightComponent>().specular;
			intensity = entity.getComponent<SpotLightComponent>().intensity;
			attenuation = entity.getComponent<SpotLightComponent>().attenuation;
			cutOff = entity.getComponent<SpotLightComponent>().cutOff;
			castShadow = entity.getComponent<SpotLightComponent>().castShadow;
		}

		mEntityStates.push_back({
			entity.id(),
			0,
			false,
			{transform.position, transform.rotation, transform.scale},
			{
				direction,
				glm::vec4(transform.position, 1.0f),
				ambient,
				diffuse,
				specular,
				attenuation,
				cutOff,
				castShadow,
				intensity
			}
		});
	}
}

void GuiSystem::render(EventBus& eventBus) {
	GuiPanels::renderGraphicsInfoPanel(mFPS);
	GuiPanels::renderPostProcessPanel(eventBus);

	for (const auto& entity: getSystemEntities()) {
		if (entity.hasComponent<InstanceComponent>())
			continue;

		if (Ui::beginEntity(entity.name())) {
			GuiPanels::renderTransformPanel(entity, eventBus, mEntityStates);
			GuiPanels::renderDebugViewsPanel(entity, eventBus, mEntityStates);
			GuiPanels::renderLightPanel(entity, eventBus, mEntityStates);
			Ui::endEntity();
		}
	}
}

void GuiSystem::updateFpsCounter(const float dt) {
	mCurrentFrameCount++;

	mCurrentSeconds += dt;
	double elapsedSeconds = mCurrentSeconds - mPreviousSeconds;
	// limit text updates to 4 per second
	if (elapsedSeconds > 0.25) {
		mPreviousSeconds = mCurrentSeconds;
		mFPS = mCurrentFrameCount / elapsedSeconds;
		mCurrentFrameCount = 0;
	}
}
