#pragma once
#include <unordered_map>
#include <vector>
#include <span>
#include "../types.hpp"

namespace Models {
class Cubemap {
public:
	explicit Cubemap(std::vector<std::string>& faces);

	~Cubemap();

	[[nodiscard]]
	MeshMap* meshes();

	[[nodiscard]]
	MaterialMap* material();

private:
	MeshMap mMeshes;
	MaterialMap mMaterial;
};
}
