#pragma once

struct DebugComponent;

struct MaterialComponent;
struct TransformComponent;

struct EntityCore {
	size_t id;
	const TransformComponent* transform{};
	const MaterialComponent* material{};
	glm::vec3 bvCenter{0.f, 0.f, 0.f};
	glm::vec3 bvExtents{0.f, 0.f, 0.f};
};