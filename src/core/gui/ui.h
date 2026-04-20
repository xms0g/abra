#pragma once
#include "glm/glm.hpp"

namespace Ui {
bool beginEntity(const char* label);

void endEntity();

void colorField4(const char* label, glm::vec4& value, float speed = 0.01f, float sameLineOffset = 100.0f);

void dragFloat3(const char* label, glm::vec3& value, float speed = 0.01f, float sameLineOffset = 100.0f);

void dragFloat4(const char* label, glm::vec4& value, float speed = 0.01f, float sameLineOffset = 100.0f);

void sliderFloat(const char* label, float* value, float sameLineOffset = 100.0f, float min = 0.0f, float max = 10.0f);
}

