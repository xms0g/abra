#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

struct Material;
class Mesh;

using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh> >;
using MaterialMap = std::unordered_map<uint32_t, Material>;

namespace Models {
class Cube {
public:
	explicit Cube(glm::vec3 color = glm::vec3(1.0f),
	              bool unlit = false,
	              const char* diffuseTexture = nullptr,
	              const char* specularTexture = nullptr,
	              const char* normalTexture = nullptr,
	              const char* heightTexture = nullptr);

	~Cube();

	[[nodiscard]] MeshMap* meshes();

	[[nodiscard]] const MaterialMap* material() const;

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
