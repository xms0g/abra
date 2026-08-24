#include "input.hpp"
#include <SDL.h>
#include "imgui/imgui_impl_sdl.h"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

bool Input::process(EventBus& eventBus, SDL_Window* window, const float dt) {
	SDL_Event event;
	bool isRunning{true};
	static bool freeLook{false};
	const ImGuiIO& io = ImGui::GetIO();

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
					isRunning = false;
				}
				break;
			case SDL_MOUSEMOTION:
				if (!io.WantCaptureMouse && freeLook) {
					eventBus.emitEvent<MouseMovementEvent>(
						static_cast<float>(event.motion.xrel),
						static_cast<float>(-event.motion.yrel)
					);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_RIGHT) {
					if (!io.WantCaptureMouse) {
						freeLook = !freeLook;
						SDL_SetRelativeMouseMode(freeLook ? SDL_TRUE : SDL_FALSE);
					}
				}
				break;
		}
	}

	const auto* keyState = SDL_GetKeyboardState(nullptr);

	if (keyState[SDL_SCANCODE_ESCAPE]) isRunning = false;
	if (keyState[SDL_SCANCODE_W]) eventBus.emitEvent<KeyPressedEvent>(Key::W, dt);
	if (keyState[SDL_SCANCODE_S]) eventBus.emitEvent<KeyPressedEvent>(Key::S, dt);
	if (keyState[SDL_SCANCODE_A]) eventBus.emitEvent<KeyPressedEvent>(Key::A, dt);
	if (keyState[SDL_SCANCODE_D]) eventBus.emitEvent<KeyPressedEvent>(Key::D, dt);

	return isRunning;
}