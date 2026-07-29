#pragma once
#include <string>
#include "glm/glm.hpp"

namespace ui {
bool beginEntity(const std::string& label);

void endEntity();

void pushID(size_t id);

void popID();

bool colorField3(const std::string& label, glm::vec3& value, float speed = 0.01f, float sameLineOffset = 100.0f);

bool dragFloat(const std::string& label, float* value, float speed = 0.01f, float sameLineOffset = 100.0f);

bool dragFloat3(const std::string& label, glm::vec3& value, float speed = 0.01f, float sameLineOffset = 100.0f);

bool dragFloat4(const std::string& label, glm::vec4& value, float speed = 0.01f, float sameLineOffset = 100.0f);

bool sliderFloat(const std::string& label, float* value, float sameLineOffset = 100.0f, float min = 0.0f, float max = 10.0f);
}

