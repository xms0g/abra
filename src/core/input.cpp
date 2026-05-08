#include "input.h"
#include <SDL.h>
#include "imgui/imgui_impl_sdl.h"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

void Input::process(EventBus& eventBus, SDL_Window* window, const float dt, bool& isRunning) {
	SDL_Event event;
	static bool freeLook{false};

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						if (event.window.windowID == SDL_GetWindowID(window)) {
							isRunning = false;
						}
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				if (freeLook) {
					eventBus.emitEvent<MouseMovementEvent>(
						static_cast<float>(event.motion.xrel),
						static_cast<float>(-event.motion.yrel)
					);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
					case SDL_BUTTON_RIGHT:
						const ImGuiIO& io = ImGui::GetIO();
						if (!io.WantCaptureMouse) {
							freeLook = !freeLook;
							SDL_SetRelativeMouseMode(freeLook ? SDL_TRUE : SDL_FALSE);
						}
						break;
				}
				break;
		}

	}

	const auto* keyState = SDL_GetKeyboardState(nullptr);
	uint32_t key{SDL_SCANCODE_UNKNOWN};

	if (keyState[SDL_SCANCODE_ESCAPE]) {
		isRunning = false;
	} else if (keyState[SDL_SCANCODE_W]) {
		key = SDL_SCANCODE_W;
	} else if (keyState[SDL_SCANCODE_S]) {
		key = SDL_SCANCODE_S;
	} else if (keyState[SDL_SCANCODE_A]) {
		key = SDL_SCANCODE_A;
	} else if (keyState[SDL_SCANCODE_D]) {
		key = SDL_SCANCODE_D;
	}

	if (key != SDL_SCANCODE_UNKNOWN) {
		eventBus.emitEvent<KeyPressedEvent>(key, dt);
	}
}
