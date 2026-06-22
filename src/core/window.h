#pragma once
#include <string>
#include <SDL.h>
#include "baseWindow.hpp"

class Window final : public BaseWindow<SDL_Window> {
public:
    Window() = default;

    ~Window() override;

    [[nodiscard]]
	SDL_GLContext glContext() const;

    void swapBuffer() override;

protected:
    void initImpl(const std::string& title, int multisamples, bool fullscreen) override;

    void clearImpl(float r, float g, float b, float a) override;

    std::string mTitle;
    SDL_GLContext mGlContext{};
};
