#pragma once
#include <unordered_map>
#include <vector>
#include <span>
#include "../types.hpp"

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
