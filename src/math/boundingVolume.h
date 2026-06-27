#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

class Mesh;

namespace math {
struct Plane;
struct Frustum;

// Base class
class BoundingVolume {
public:
	BoundingVolume() = default;
	BoundingVolume(const glm::vec3& min, const glm::vec3& max)
		: mCenter{(max + min) * 0.5f}, mExtents{max.x - mCenter.x, max.y - mCenter.y, max.z - mCenter.z} {
	}

	BoundingVolume(const glm::vec3& inCenter, const float iI, const float iJ, const float iK)
		: mCenter{inCenter}, mExtents{iI, iJ, iK} {
	}

	virtual ~BoundingVolume() = default;

	[[nodiscard]]
	glm::vec3 center() const {
		return mCenter;
	}

	[[nodiscard]]
	glm::vec3 extents() const {
		return mExtents;
	}

protected:
	[[nodiscard]]
	virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;

	glm::vec3 mCenter{0.f};
	glm::vec3 mExtents{0.f};
};

// Dummy
class DummyBV final: public BoundingVolume {
	public:
		~DummyBV() override = default;

protected:
	[[nodiscard]]
	bool isOnOrForwardPlane(const Plane& plane) const override {
		return true;
	}
};

// Sphere
class Sphere final : public BoundingVolume {
public:
	Sphere(const glm::vec3& center, const float radius)
		: BoundingVolume(center, 0.0f, 0.0f, 0.0f), mRadius(radius) {
	}

	~Sphere() override = default;

	[[nodiscard]]
	bool isOnFrustum(const Frustum& camFrustum, const glm::mat4& model) const;

	[[nodiscard]]
	static bool isMeshInFrustum(
		const Frustum& camFrustum,
		const glm::vec3& min,
		const glm::vec3& max,
		const glm::mat4& model);

protected:
	[[nodiscard]]
	bool isOnOrForwardPlane(const Plane& plane) const override;

	float mRadius{0.f};
};

// AABB
class AABB final : public BoundingVolume {
public:
	AABB(const glm::vec3& min, const glm::vec3& max)
		: BoundingVolume(min, max) {
	}

	AABB(const glm::vec3& inCenter, const float iI, const float iJ, const float iK)
		: BoundingVolume(inCenter, iI, iJ, iK) {
	}

	~AABB() override = default;

	[[nodiscard]]
	static bool isOnFrustum(
		const Frustum& camFrustum,
		const glm::mat4& model,
		const glm::vec3& center,
		const glm::vec3& extents);

	[[nodiscard]]
	static bool isMeshInFrustum(
		const Frustum& camFrustum,
		const glm::vec3& min,
		const glm::vec3& max,
		const glm::mat4& model);

protected:
	[[nodiscard]]
	bool isOnOrForwardPlane(const Plane& plane) const override;
};

Sphere generateSphereBV(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID);

AABB generateAABB(const std::unordered_map<uint32_t, std::vector<Mesh> >& meshesByMatID);
}
