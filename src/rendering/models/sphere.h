#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "../types.hpp"

namespace Model {
class Sphere {
public:
	explicit Sphere(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const std::string& albedo = "",
		const std::string& normal = "",
		const std::string& orm = "");

	~Sphere();

	[[nodiscard]]
	MeshMap& meshes();

	[[nodiscard]]
	MaterialMap& material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
