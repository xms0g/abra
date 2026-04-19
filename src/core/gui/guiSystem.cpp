#include "guiSystem.h"
#include "guiPanels.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/debug.hpp"
#include "../../ECS/components/directionalLight.hpp"
#include "../../ECS/components/pointLight.hpp"
#include "../../ECS/components/spotLight.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../ECS/components/instance.hpp"

GuiSystem::GuiSystem(SDL_Window* window, const SDL_GLContext context) {
	RequireComponent<TransformComponent>(true);
	RequireComponent<PointLightComponent>(true);
	RequireComponent<SpotLightComponent>(true);
	RequireComponent<DirectionalLightComponent>(true);
	RequireComponent<DebugComponent>(true);
	// Initialize the ImGui context
	GuiBackend::init(window, context, "#version 410");
}

GuiSystem::~GuiSystem() {
	// Clean up ImGui
	GuiBackend::shutdown();
}

void GuiSystem::update(const float dt) {
	updateFpsCounter(dt);
}

void GuiSystem::render(const std::vector<std::shared_ptr<IPostEffect>>& effects) const {
	// Start the Dear ImGui frame
	GuiBackend::newFrame();

	GuiPanels::renderGraphicsInfoPanel(mFPS);
	GuiPanels::renderPostProcessPanel(effects);

	for (const auto& entity: getSystemEntities()) {
		if (entity.hasComponent<InstanceComponent>()) continue;

		if (Ui::beginEntity(entity.name())) {
			GuiPanels::renderTransformPanel(entity);
			GuiPanels::renderDebugViewsPanel(entity);
			Ui::endEntity();
		}

		if (entity.hasComponent<DirectionalLightComponent>() ||
			entity.hasComponent<PointLightComponent>() ||
			entity.hasComponent<SpotLightComponent>()) {
			GuiPanels::renderLightPanel(entity);
		}
	}

	//Render ImGui
	GuiBackend::renderFrame();
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

