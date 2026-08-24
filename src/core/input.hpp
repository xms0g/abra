#pragma once

class EventBus;
class SDL_Window;

namespace Input {
bool process(EventBus& eventBus, SDL_Window* window, float dt);
};
