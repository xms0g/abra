#include "engine.h"
#include <iostream>
#include "glm/glm.hpp"
#include "input.h"
#include "camera.h"
#include "window.h"
#include "gui/guiSystem.h"
#include "../ECS/registry.h"
#include "../rendering/renderPipeline.h"
#include "../rendering/renderPasses/postProcess/postProcessPass.h"

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::init(Registry* registry) {
	mRegistry = registry;

	mWindow = std::make_unique<Window>();
	mWindow->init("Abra");

	registry->addSystem<GuiSystem>(mWindow->nativeHandle(), mWindow->glContext());
	mGuiSystem = &registry->getSystem<GuiSystem>();

	registry->addSystem<RenderPipeline>(mRegistry);
	mRenderPipeline = &registry->getSystem<RenderPipeline>();

	mCamera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f));
	mInput = std::make_unique<Input>();
}

void Engine::configure() const {
	mRenderPipeline->batchEntities();
	mRenderPipeline->configure(*mCamera);
}

void Engine::run() {
	while (isRunning) {
		mWindow->clear(0.0f, 0.0f, 0.0f, 1.0f);

		mDeltaTime = (SDL_GetTicks() - mMillisecsPreviousFrame) / 1000.0f;
		mMillisecsPreviousFrame = SDL_GetTicks();

		mInput->process(*mCamera, mWindow->nativeHandle(), mDeltaTime, isRunning);

		mRenderPipeline->render();
#ifdef DEBUG
		mGuiSystem->update(mDeltaTime);
		mGuiSystem->render(mRenderPipeline->postProcess().effects());
#endif
		// SDL swap buffers
		mWindow->swapBuffer();
	}
}
