#include "input.h"
#include <SDL.h>
#include "imgui/imgui_impl_sdl.h"
#include "camera.h"

void Input::process(Camera& camera, SDL_Window* window, const float dt, bool& isRunning) {
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

	processKeyboard(camera, dt, isRunning);
	processMouse(camera);
	camera.update();
}

void Input::processKeyboard(Camera& camera, const float dt, bool& isRunning) {
	const auto* keyState = SDL_GetKeyboardState(nullptr);

	if (keyState[SDL_SCANCODE_ESCAPE]) {
		isRunning = false;
	} else if (keyState[SDL_SCANCODE_W]) {
		camera.processKeyboard(FORWARD, dt);
	} else if (keyState[SDL_SCANCODE_S]) {
		camera.processKeyboard(BACKWARD, dt);
	} else if (keyState[SDL_SCANCODE_A]) {
		camera.processKeyboard(LEFT, dt);
	} else if (keyState[SDL_SCANCODE_D]) {
		camera.processKeyboard(RIGHT, dt);
	}
}

void Input::processMouse(Camera& camera) {
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
		camera.processMouseMovement(static_cast<float>(dx), static_cast<float>(-dy));
	}
}
