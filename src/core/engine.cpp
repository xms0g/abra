#include "engine.hpp"
#include <iostream>
#include "glm/glm.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "window.hpp"
#include "gui/system.hpp"
#include "../event/eventBus.hpp"
#include "../ECS/registry.hpp"
#include "../rendering/renderer.hpp"
#include "../config/configManager.hpp"

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::init(Registry& registry) {
	mCamera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f));
	mEventBus = std::make_unique<EventBus>();

	mWindow = std::make_unique<Window>(
		CONFIG_MANAGER.get<std::string>("window.title"),
		CONFIG_MANAGER.get<int32_t>("msaa.samples"),
		CONFIG_MANAGER.get<bool>("window.fullscreen"));

	mGuiSystem = &registry.addSystem<GuiSystem>();
	mRenderer = &registry.addSystem<Renderer>(registry, *mCamera, *mWindow);
}

void Engine::configure() const {
	mCamera->configure(*mEventBus);
	mRenderer->configure(*mEventBus);
}

void Engine::run() {
	while (isRunning) {
		mWindow->clear(0.0f, 0.0f, 0.0f, 1.0f);

		mDeltaTime = static_cast<float>(SDL_GetTicks() - mMillisecsPreviousFrame) / 1000.0f;
		mMillisecsPreviousFrame = SDL_GetTicks();

		Input::process(*mEventBus, &**mWindow, mDeltaTime, isRunning);
		mCamera->update();
		mGuiSystem->update(mDeltaTime);

		mRenderer->render();
		mGuiSystem->render(*mEventBus);
		Renderer::drawGui();

		// SDL swap buffers
		mWindow->swapBuffer();
	}
}
