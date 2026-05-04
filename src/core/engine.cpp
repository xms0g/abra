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

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::init(Registry* registry) {
	mRegistry = registry;

	mWindow = std::make_unique<Window>();
	mWindow->init("Abra");

	registry->addSystem<GuiSystem>();
	mGuiSystem = &registry->getSystem<GuiSystem>();

	registry->addSystem<RenderPipeline>(mRegistry, mWindow->nativeHandle(), mWindow->glContext());
	mRenderPipeline = &registry->getSystem<RenderPipeline>();

	mCamera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f));
	mInput = std::make_unique<Input>();
	mEventBus = std::make_unique<EventBus>();

	mCamera->subscribeToEvents(*mEventBus);
}

void Engine::configure() const {
	mRenderPipeline->batchEntities();
	mRenderPipeline->configure(*mCamera, *mEventBus);
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
