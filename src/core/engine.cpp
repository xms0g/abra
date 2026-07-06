#include "engine.h"
#include <iostream>
#include "glm/glm.hpp"
#include "input.h"
#include "camera.h"
#include "window.h"
#include "gui/system.h"
#include "../event/eventBus.hpp"
#include "../ECS/registry.h"
#include "../rendering/renderPipeline.h"
#include "../config/configManager.h"

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::init(Registry& registry) {
	mCamera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f));
	mEventBus = std::make_unique<EventBus>();

	mWindow = std::make_unique<Window>(
		cfg.get<std::string>("window.title"),
		cfg.get<int32_t>("msaa.sample_count"),
		cfg.get<bool>("window.fullscreen"));

	mGuiSystem = &registry.addSystem<GuiSystem>();
	mRenderPipeline = &registry.addSystem<RenderPipeline>(registry, *mCamera, mWindow->nativeHandle(), mWindow->glContext());
}

void Engine::configure() const {
	mCamera->configure(*mEventBus);
	mRenderPipeline->configure(*mCamera, *mEventBus);
}

void Engine::run() {
	while (isRunning) {
		mWindow->clear(0.0f, 0.0f, 0.0f, 1.0f);

		mDeltaTime = static_cast<float>(SDL_GetTicks() - mMillisecsPreviousFrame) / 1000.0f;
		mMillisecsPreviousFrame = SDL_GetTicks();

		Input::process(*mEventBus, mWindow->nativeHandle(), mDeltaTime, isRunning);
		mCamera->update();
		mGuiSystem->update(mDeltaTime);

		mRenderPipeline->render();
		mGuiSystem->render(*mEventBus);
		RenderPipeline::drawGui();

		// SDL swap buffers
		mWindow->swapBuffer();
	}
}
