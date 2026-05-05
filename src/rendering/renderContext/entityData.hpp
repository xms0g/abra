#pragma once

struct MaterialComponent;
struct TransformComponent;

struct EntityCore {
	size_t id;
	uint32_t debugMode;

	struct {
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
	};

	const TransformComponent* transform{};
	const MaterialComponent* material{};
	glm::vec3 bvCenter{0.f, 0.f, 0.f};
	glm::vec3 bvExtents{0.f, 0.f, 0.f};
};
