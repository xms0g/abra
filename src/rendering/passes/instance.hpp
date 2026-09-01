#pragma once
#include "glm/glm.hpp"

struct InstanceData {
	glm::mat4 modelMatrix{};
	glm::mat3 normalMatrix{};
	float padding[3]{};
};
