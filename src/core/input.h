#pragma once

class Camera;
class SDL_Window;

class Input {
public:
    Input() = default;

    void process(Camera& camera, SDL_Window* window, float dt, bool& isRunning);

private:
    void processKeyboard(Camera& camera, float dt, bool& isRunning);

    void processMouse(Camera& camera);
};
