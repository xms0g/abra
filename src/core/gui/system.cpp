#include "system.h"
#include "panels.h"
#include "ui.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/material.hpp"
#include "../../ECS/components/instance.hpp"
#include "../../rendering/material/material.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"

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

void GuiSystem::render(EventBus& eventBus) const {
	GuiPanels::renderGraphicsInfoPanel(mFPS);
	GuiPanels::renderPostProcessPanel(eventBus);

	for (const auto& entity: getSystemEntities()) {
		uint32_t lightIdx{0};
		bool lightChanged{false}, transformChanged{false};
		auto& transform = entity.getComponent<TransformComponent>();

		if (ui::beginEntity(entity.name())) {
			GuiPanels::renderTransformPanel(entity, transform, transformChanged);
			GuiPanels::renderDebugViewsPanel(entity, eventBus);
			GuiPanels::renderLightPanel(entity, lightChanged, lightIdx);
			ui::endEntity();
		}

		if (transformChanged) {
			eventBus.emitEvent<GuiTransformEvent>(entity.id(), transform.position, transform.rotation, transform.scale);
		}

		const bool isLight = entity.hasComponent<PointLightComponent>() ||
		                     entity.hasComponent<SpotLightComponent>() ||
		                     entity.hasComponent<DirectionalLightComponent>();

		if (lightChanged || (isLight && transformChanged)) {
			uint32_t matIdx = entity.getComponent<MaterialComponent>().materials[0].at(0).idx;
			eventBus.emitEvent<GuiLightEvent>(entity.id(), matIdx, lightIdx);
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
