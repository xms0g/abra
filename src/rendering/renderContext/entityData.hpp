#pragma once

struct DebugComponent;

namespace math {
class BoundingVolume;
}

struct MaterialComponent;
struct TransformComponent;
class Entity;

struct EntityData {
	const DebugComponent* debug{};
	const TransformComponent* transform{};
	const MaterialComponent* material{};
	const math::BoundingVolume* bv{};
};