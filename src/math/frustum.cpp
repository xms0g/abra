#include "frustum.hpp"

math::Plane::Plane(const glm::vec3& p1, const glm::vec3& norm): normal(norm) {
	normal = glm::normalize(normal);
	distance = glm::dot(p1, normal);
}

float math::Plane::computeSignedDistanceToPlane(const glm::vec3& point) const {
	return glm::dot(normal, point) - distance;
}
