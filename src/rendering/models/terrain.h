#pragma once
#include "glm/glm.hpp"
#include "../types.hpp"


namespace Model {
class Terrain {
public:
	explicit Terrain(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const char* diffuseTexture = nullptr,
		const char* specularTexture = nullptr,
		const char* normalTexture = nullptr,
		const char* heightTexture = nullptr);

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
