#pragma once

class EventBus;
class SDL_Window;

class Input {
public:
    Input() = default;

    void process(EventBus& eventBus, SDL_Window* window, float dt, bool& isRunning);

private:
    void processKeyboard(EventBus& eventBus, float dt, bool& isRunning);

    void processMouse(EventBus& eventBus);
};
