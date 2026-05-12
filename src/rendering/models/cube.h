#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "../types.hpp"

namespace Models {
class Cube {
public:
	explicit Cube(
		glm::vec3 color = glm::vec3(1.0f),
		bool unlit = false,
		const char* diffuseTexture = nullptr,
		const char* specularTexture = nullptr,
		const char* normalTexture = nullptr,
		const char* heightTexture = nullptr);

	~Cube();

	[[nodiscard]]
	MeshMap* meshes();

	[[nodiscard]]
	MaterialMap* material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
