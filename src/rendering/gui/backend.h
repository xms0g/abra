#pragma once
#include <SDL.h>
#include "glm/glm.hpp"

namespace GuiBackend {
void init(SDL_Window* window, SDL_GLContext context, const char* glsl_version);

void shutdown();

void newFrame();

void renderFrame();
}