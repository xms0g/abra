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

}

void Input::processKeyboard(Camera& camera, const float dt, bool& isRunning) {
    auto* keyState = SDL_GetKeyboardState(nullptr);

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

    if (const uint32_t buttons = SDL_GetRelativeMouseState(&dx, &dy); buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		freeLook = !freeLook;
	}

#ifdef DEBUG
    ImGuiIO& io = ImGui::GetIO();
	if (!freeLook) {
		// Only feed ImGui when camera is NOT controlling mouse
		int x, y;
		SDL_GetMouseState(&x, &y);
		io.MousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
		io.MouseDown[1] = false;
	}
#endif

	if (freeLook) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		camera.processMouseMovement(static_cast<float>(dx), static_cast<float>(-dy));
	} else {
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}
