#pragma once
#include <unordered_map>
#include <vector>
#include <span>

struct Material;
class Mesh;
struct Texture;

using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh>>;
using MaterialMap = std::unordered_map<uint32_t, Material>;

namespace Models {
class Cubemap {
public:
	explicit Cubemap(std::span<const char* const> faces);

	~Cubemap();

	[[nodiscard]] MeshMap* meshes();

	[[nodiscard]] const MaterialMap* material() const;

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
