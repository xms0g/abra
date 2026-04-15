#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

class Mesh;

namespace math {
struct Plane;
struct Frustum;

class BoundingVolume {
public:
	virtual ~BoundingVolume() = default;

	[[nodiscard]] virtual bool isOnFrustum(const Frustum& camFrustum, const glm::mat4& model) const = 0;

	[[nodiscard]] virtual bool isMeshInFrustum(
		const Frustum& camFrustum,
		const glm::vec3& min,
		const glm::vec3& max,
		const glm::mat4& model) const = 0;

protected:
	[[nodiscard]] virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;
};

// Sphere
class Sphere final : public BoundingVolume {
public:
	Sphere() = default;

	Sphere(const glm::vec3& center, const float radius) : mCenter(center), mRadius(radius) {
	}

	~Sphere() override = default;

	[[nodiscard]] bool isOnFrustum(const Frustum& camFrustum, const glm::mat4& model) const override;

	[[nodiscard]] bool isMeshInFrustum(
		const Frustum& camFrustum,
		const glm::vec3& min,
		const glm::vec3& max,
		const glm::mat4& model) const override;

protected:
	[[nodiscard]] bool isOnOrForwardPlane(const Plane& plane) const override;

	glm::vec3 mCenter{0.f, 0.f, 0.f};
	float mRadius{0.f};
};

// AABB
class AABB final : public BoundingVolume {
public:
	AABB() = default;

	AABB(const glm::vec3& min, const glm::vec3& max)
		: center{(max + min) * 0.5f},
		  extents{max.x - center.x, max.y - center.y, max.z - center.z} {
	}

	AABB(const glm::vec3& inCenter, const float iI, const float iJ, const float iK)
		: center{inCenter}, extents{iI, iJ, iK} {
	}

	~AABB() override = default;

	[[nodiscard]] bool isOnFrustum(const Frustum& camFrustum, const glm::mat4& model) const override;

	[[nodiscard]] bool isMeshInFrustum(
		const Frustum& camFrustum,
		const glm::vec3& min,
		const glm::vec3& max,
		const glm::mat4& model) const override;

protected:
	[[nodiscard]] bool isOnOrForwardPlane(const Plane& plane) const override;

	glm::vec3 center{0.f, 0.f, 0.f};
	glm::vec3 extents{0.f, 0.f, 0.f};
};

Sphere generateSphereBV(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID);

AABB generateAABB(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID);
}
