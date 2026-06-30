#pragma once

class EventBus;
class SDL_Window;

namespace Input {
void process(EventBus& eventBus, SDL_Window* window, float dt, bool& isRunning);
};
