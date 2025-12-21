#pragma once

struct DebugComponent;

namespace math {
class BoundingVolume;
}

struct MaterialComponent;
struct TransformComponent;

struct EntityData {
	const DebugComponent* debug{};
	const TransformComponent* transform{};
	const MaterialComponent* material{};
	const math::BoundingVolume* bv{};
};