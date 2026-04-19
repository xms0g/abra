#pragma once

struct DebugComponent;
struct MaterialComponent;
struct TransformComponent;

struct EntityCore {
	const DebugComponent* debug{};
	glm::vec3 position{0.f};
	glm::vec3 rotation{0.f};
	glm::vec3 scale{1.f};
	float heightScale{1.f};
	glm::vec3 bvCenter{0.f};
	glm::vec3 bvExtents{0.f};
};