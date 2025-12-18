#include "input.h"
#include <SDL.h>
#include "imgui/imgui_impl_sdl.h"
#include "camera.h"

void Input::process(Camera& camera, SDL_Window* window, const float dt, bool& isRunning) {
    SDL_Event event;
    SDL_PollEvent(&event);

#ifdef DEBUG
    ImGui_ImplSDL2_ProcessEvent(&event);
#endif

    if (event.type == SDL_QUIT)
        isRunning = false;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
        isRunning = false;

    processKeyboard(camera, dt, isRunning);
    processMouse(camera);
	camera.update();
}

void Input::processKeyboard(Camera& camera, const float dt, bool& isRunning) {
	if (auto* keyState = SDL_GetKeyboardState(nullptr); keyState[SDL_SCANCODE_ESCAPE]) {
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

	if (rmb && !prevRMB) {
		freeLook = !freeLook;
		SDL_SetRelativeMouseMode(freeLook ? SDL_TRUE : SDL_FALSE);
	}
	prevRMB = rmb;

#ifdef DEBUG
    ImGuiIO& io = ImGui::GetIO();
	if (!freeLook) {
		// Only feed ImGui when camera is NOT controlling mouse
		int x, y;
		SDL_GetMouseState(&x, &y);
		io.MousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
		io.MouseDown[1] = rmb;
	}
#endif

	if (freeLook) {
		camera.processMouseMovement(static_cast<float>(dx), static_cast<float>(-dy));
	}
}
