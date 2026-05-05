#pragma once
#include <cstdint>
#include "glm/glm.hpp"

struct EntityState {
	size_t id{0};
	uint32_t debugMode{0};

	struct {
		glm::vec3 position{0.0f};
		glm::vec3 rotation{0.0f};
		glm::vec3 scale{1.0f};
	} transform;
};
