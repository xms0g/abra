#pragma once
#include <SDL.h>
#include "glm/glm.hpp"

namespace GuiBackend {
void init(SDL_Window* window, SDL_GLContext context, const char* glsl_version);

void shutdown();

void newFrame();

void renderFrame();
}

namespace Ui {
bool beginEntity(const char* label);

void endEntity();

void colorField4(const char* label, glm::vec4& value, float speed = 0.01f, float sameLineOffset = 100.0f);

void dragFloat3(const char* label, glm::vec3& value, float speed = 0.01f, float sameLineOffset = 100.0f);

void dragFloat4(const char* label, glm::vec4& value,  float speed = 0.01f, float sameLineOffset = 100.0f);

void sliderFloat(const char* label, float* value, float sameLineOffset = 100.0f,float min = 0.0f, float max = 10.0f);
}
