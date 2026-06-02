#pragma once
#include <vector>
#include <unordered_map>

class Mesh;
struct MeshComponent {
	using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh>>;
	const MeshMap* meshes;

	MeshComponent() = default;
	explicit MeshComponent(const MeshMap* m) : meshes(m) {}
};