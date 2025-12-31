#pragma once
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

struct Material;
class Mesh;
struct Vertex;

using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh> >;
using MaterialMap = std::unordered_map<uint32_t, Material>;

namespace Models {
class Plane {
public:
	explicit Plane(glm::vec3 color,
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
