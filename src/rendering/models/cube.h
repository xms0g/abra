#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "../types.hpp"

namespace Model {
class Cube {
public:
	explicit Cube(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const std::string& diffuseTexture = "",
		const std::string& specularTexture = "",
		const std::string& normalTexture = "",
		const std::string& heightTexture = "");

	~Cube();

	[[nodiscard]]
	MeshMap& meshes();

	[[nodiscard]]
	MaterialMap& material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
