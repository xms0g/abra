#include "boundingVolume.h"
#include <algorithm>
#include "frustum.h"
#include "../rendering/mesh/mesh.h"
#include "../rendering/mesh/vertex.hpp"

bool math::Sphere::isOnFrustum(const Frustum& camFrustum, const glm::mat4& model) const {
	const glm::vec3 globalScale = {
		glm::length(model[0]),
		glm::length(model[1]),
		glm::length(model[2])
	};

	//Get our global center with process it with the global model matrix of our transform
	const glm::vec3 globalCenter{model * glm::vec4(mCenter, 1.f)};

	//To wrap correctly our shape, we need the maximum scale scalar.
	const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

	//Max scale is assuming for the diameter. So, we need the half to apply it to our radius
	const Sphere globalSphere(globalCenter, mRadius * (maxScale * 0.5f));

	//Check Firstly the result that have the most chance to failure to avoid to call all functions.
	return globalSphere.isOnOrForwardPlane(camFrustum.leftFace) &&
	       globalSphere.isOnOrForwardPlane(camFrustum.rightFace) &&
	       globalSphere.isOnOrForwardPlane(camFrustum.farFace) &&
	       globalSphere.isOnOrForwardPlane(camFrustum.nearFace) &&
	       globalSphere.isOnOrForwardPlane(camFrustum.topFace) &&
	       globalSphere.isOnOrForwardPlane(camFrustum.bottomFace);
}

bool math::Sphere::isMeshInFrustum(
	const Frustum& camFrustum,
	const glm::vec3& min,
	const glm::vec3& max,
	const glm::mat4& model) {
	return false;
}

bool math::Sphere::isOnOrForwardPlane(const Plane& plane) const {
	return plane.computeSignedDistanceToPlane(mCenter) > -mRadius;
}

bool math::AABB::isOnFrustum(const Frustum& camFrustum,
                             const glm::mat4& model,
                             const glm::vec3& center,
                             const glm::vec3& extents) {
	const glm::vec3 globalCenter{model * glm::vec4(center, 1.f)};
	// Scaled orientation
	const glm::vec3 right = model[0] * extents.x;
	const glm::vec3 up = model[1] * extents.y;
	const glm::vec3 forward = -model[2] * extents.z;

	const float newIi = std::abs(right.x) + std::abs(up.x) + std::abs(forward.x);
	const float newIj = std::abs(right.y) + std::abs(up.y) + std::abs(forward.y);
	const float newIk = std::abs(right.z) + std::abs(up.z) + std::abs(forward.z);

	const AABB globalAABB{globalCenter, newIi, newIj, newIk};

	return globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
	       globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
	       globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
	       globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
	       globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
	       globalAABB.isOnOrForwardPlane(camFrustum.farFace);
}

bool math::AABB::isMeshInFrustum(const Frustum& camFrustum,
                                 const glm::vec3& min,
                                 const glm::vec3& max,
                                 const glm::mat4& model) {
	// Define local corners
	const glm::vec3 localCorners[8] = {
		{min.x, min.y, min.z}, {max.x, min.y, min.z},
		{min.x, max.y, min.z}, {max.x, max.y, min.z},
		{min.x, min.y, max.z}, {max.x, min.y, max.z},
		{min.x, max.y, max.z}, {max.x, max.y, max.z}
	};

	// Transform to world space immediately
	glm::vec3 worldCorners[8];
	for (uint32_t i = 0; i < 8; ++i) {
		worldCorners[i] = glm::vec3(model * glm::vec4(localCorners[i], 1.0f));
	}

	const Plane planes[6] = {
		camFrustum.topFace, camFrustum.bottomFace,
		camFrustum.rightFace, camFrustum.leftFace,
		camFrustum.farFace, camFrustum.nearFace
	};

	for (const auto& plane: planes) {
		uint32_t inside = 0;

		for (const auto& corner: worldCorners) {
			if (plane.computeSignedDistanceToPlane(corner) >= 0) {
				++inside;
			}
		}

		// If inside is 0, it means ALL corners are behind this specific plane.
		// Therefore, the object is outside the frustum.
		if (inside == 0)
			return false;
	}
	return true;
}

bool math::AABB::isOnOrForwardPlane(const Plane& plane) const {
	const float r = mExtents.x * std::abs(plane.normal.x) +
	                mExtents.y * std::abs(plane.normal.y) +
	                mExtents.z * std::abs(plane.normal.z);

	return -r <= plane.computeSignedDistanceToPlane(mCenter);
}

math::Sphere math::generateSphereBV(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID) {
	auto minAABB = glm::vec3(std::numeric_limits<float>::max());
	auto maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (auto& [matID, meshes]: meshesByMatID) {
		for (auto& mesh: meshes) {
			for (auto& vertex: mesh.vertices()) {
				minAABB.x = std::min(minAABB.x, vertex.position.x);
				minAABB.y = std::min(minAABB.y, vertex.position.y);
				minAABB.z = std::min(minAABB.z, vertex.position.z);

				maxAABB.x = std::max(maxAABB.x, vertex.position.x);
				maxAABB.y = std::max(maxAABB.y, vertex.position.y);
				maxAABB.z = std::max(maxAABB.z, vertex.position.z);
			}
		}
	}

	return {(maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB)};
}

math::AABB math::generateAABB(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID) {
	auto minAABB = glm::vec3(std::numeric_limits<float>::max());
	auto maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (auto& [matID, meshes]: meshesByMatID) {
		for (auto& mesh: meshes) {
			for (auto& vertex: mesh.vertices()) {
				minAABB.x = std::min(minAABB.x, vertex.position.x);
				minAABB.y = std::min(minAABB.y, vertex.position.y);
				minAABB.z = std::min(minAABB.z, vertex.position.z);

				maxAABB.x = std::max(maxAABB.x, vertex.position.x);
				maxAABB.y = std::max(maxAABB.y, vertex.position.y);
				maxAABB.z = std::max(maxAABB.z, vertex.position.z);
			}
		}
	}

	return {minAABB, maxAABB};
}
