#include "system.h"
#include "panels.h"
#include "ui.h"
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

void GuiSystem::render(EventBus& eventBus) const {
	GuiPanels::renderGraphicsInfoPanel(mFPS);
	GuiPanels::renderPostProcessPanel(eventBus);

	for (const auto& entity: getSystemEntities()) {
		if (entity.hasComponent<InstanceComponent>())
			continue;

		if (Ui::beginEntity(entity.name())) {
			GuiPanels::renderTransformPanel(entity, eventBus);
			GuiPanels::renderDebugViewsPanel(entity, eventBus);
			GuiPanels::renderLightPanel(entity, eventBus);
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
