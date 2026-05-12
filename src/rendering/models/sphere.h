#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "../types.hpp"

namespace Models {
class Sphere {
public:
	explicit Sphere(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const char* albedo = nullptr,
		const char* normal = nullptr,
		const char* orm = nullptr);

	~Sphere();

	[[nodiscard]]
	MeshMap* meshes();

	[[nodiscard]]
	MaterialMap* material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
