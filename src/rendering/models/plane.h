#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "../types.hpp"

namespace Models {
class Plane {
public:
	explicit Plane(
		glm::vec3 color,
		const char* diffuseTexture = nullptr,
		const char* specularTexture = nullptr,
		const char* normalTexture = nullptr,
		const char* heightTexture = nullptr);

	~Plane();

	[[nodiscard]] MeshMap* meshes();

	[[nodiscard]] const MaterialMap* material() const;

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
