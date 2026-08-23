#pragma once
#include "glm/glm.hpp"

namespace math {
struct Plane {
	glm::vec3 normal{ 0.f, 1.f, 0.f };
	float distance{0.0f};

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm);

	[[nodiscard]]
	float computeSignedDistanceToPlane(const glm::vec3& point) const;
};

struct Frustum {
	Plane topFace;
	Plane bottomFace;

	Plane rightFace;
	Plane leftFace;

	Plane farFace;
	Plane nearFace;
};

}
