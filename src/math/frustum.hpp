#pragma once
#include "glm/glm.hpp"

namespace math {
struct Plane {
	glm::vec3 normal{ 0.f, 1.f, 0.f };
	float distance{0.0f};

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm) : normal(norm) {
		normal = glm::normalize(normal);
		distance = glm::dot(p1, normal);
	}

	[[nodiscard]] float computeSignedDistanceToPlane(const glm::vec3& point) const {
		return glm::dot(normal, point) - distance;
	}
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
