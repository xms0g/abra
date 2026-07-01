#pragma once
#include "glm/glm.hpp"
#include "../types.hpp"


namespace Model {
class Terrain {
public:
	explicit Terrain(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const std::string& diffuseTexture = "",
		const std::string& specularTexture = "",
		const std::string& normalTexture = "",
		const std::string& heightTexture = "");

	~Terrain();

	[[nodiscard]]
	MeshMap& meshes();

	[[nodiscard]]
	MaterialMap& material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
