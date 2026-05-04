#include "input.h"
#include <SDL.h>
#include "imgui/imgui_impl_sdl.h"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseEvent.hpp"

void Input::process(EventBus& eventBus, SDL_Window* window, const float dt, bool& isRunning) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		if (event.type == SDL_QUIT)
			isRunning = false;
		if (event.type == SDL_WINDOWEVENT &&
		    event.window.event == SDL_WINDOWEVENT_CLOSE &&
		    event.window.windowID == SDL_GetWindowID(window)) {
			isRunning = false;
		}
	}

	processKeyboard(eventBus, dt, isRunning);
	processMouse(eventBus);
}

void Input::processKeyboard(EventBus& eventBus, const float dt, bool& isRunning) {
	const auto* keyState = SDL_GetKeyboardState(nullptr);

	if (keyState[SDL_SCANCODE_ESCAPE]) {
		isRunning = false;
	} else if (keyState[SDL_SCANCODE_W]) {
		eventBus.emitEvent<KeyPressedEvent>(SDL_SCANCODE_W, dt);
	} else if (keyState[SDL_SCANCODE_S]) {
		eventBus.emitEvent<KeyPressedEvent>(SDL_SCANCODE_S, dt);
	} else if (keyState[SDL_SCANCODE_A]) {
		eventBus.emitEvent<KeyPressedEvent>(SDL_SCANCODE_A, dt);
	} else if (keyState[SDL_SCANCODE_D]) {
		eventBus.emitEvent<KeyPressedEvent>(SDL_SCANCODE_D, dt);
	}
}

void Input::processMouse(EventBus& eventBus) {
	int dx, dy;
	static bool freeLook{false};
	static bool prevRMB{false};

	const uint32_t buttons = SDL_GetRelativeMouseState(&dx, &dy);
	const bool rmb = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

	ImGuiIO& io = ImGui::GetIO();

	if (rmb && !prevRMB && !io.WantCaptureMouse) {
		freeLook = !freeLook;
		SDL_SetRelativeMouseMode(freeLook ? SDL_TRUE : SDL_FALSE);
	}
	prevRMB = rmb;

	if (freeLook) {
		eventBus.emitEvent<MouseEvent>(static_cast<float>(dx), static_cast<float>(-dy));
	}
}
