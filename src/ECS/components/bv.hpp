#pragma once
#include <memory>

namespace math {
class BoundingVolume;
}

struct BoundingVolumeComponent {
	std::shared_ptr<math::BoundingVolume> bv;

	BoundingVolumeComponent() = default;
	explicit BoundingVolumeComponent(const std::shared_ptr<math::BoundingVolume>& b) : bv(b) {}
};
