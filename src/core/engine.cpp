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

void Engine::init(Registry* registry) {
	mRegistry = registry;

	mWindow = std::make_unique<Window>();
	mWindow->init(
		cfg.get<std::string>("window.title"),
		cfg.get<int32_t>("msaa.sample_count"),
		cfg.get<bool>("window.fullscreen"));

	cfg.set<int32_t>("window.width", mWindow->width());
	cfg.set<int32_t>("window.height", mWindow->height());

	mGuiSystem = &registry->addSystem<GuiSystem>();
	mRenderPipeline = &registry->addSystem<RenderPipeline>(mRegistry, mWindow->nativeHandle(), mWindow->glContext());

	mCamera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f));
	mInput = std::make_unique<Input>();
	mEventBus = std::make_unique<EventBus>();
}

void Engine::configure() const {
	mCamera->configure(*mEventBus);
	mRenderPipeline->batchEntities();
	mRenderPipeline->configure(*mCamera, *mEventBus);
	mGuiSystem->configure();
}

void Engine::run() {
	while (isRunning) {
		mWindow->clear(0.0f, 0.0f, 0.0f, 1.0f);

		mDeltaTime = static_cast<float>(SDL_GetTicks() - mMillisecsPreviousFrame) / 1000.0f;
		mMillisecsPreviousFrame = SDL_GetTicks();

		mInput->process(*mEventBus, mWindow->nativeHandle(), mDeltaTime, isRunning);
		mCamera->update();
		mGuiSystem->update(mDeltaTime);

		mRenderPipeline->render();
		mGuiSystem->render(*mEventBus);
		mRenderPipeline->drawGui();

		// SDL swap buffers
		mWindow->swapBuffer();
	}
}
