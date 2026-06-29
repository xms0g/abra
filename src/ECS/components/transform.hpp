#pragma once
#include "glm/glm.hpp"

struct TransformComponent {
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{};

	bool isDirty;

	TransformComponent() = default;

	explicit TransformComponent(
		const glm::vec3 position,
		const glm::vec3 rotation,
		const glm::vec3 scale)
		: position(position),
		  rotation(rotation),
		  scale(scale),
		  isDirty(false) {
	}
};
