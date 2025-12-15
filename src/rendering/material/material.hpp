#pragma once
#include <cstdint>
#include <vector>
#include "glm/glm.hpp"

enum MaterialFlags: uint32_t {
	Opaque = 1 << 0,
	Blend = 1 << 1,
	Cutout = 1 << 2,
	CastShadow = 1 << 3,
	TwoSided = 1 << 4
};

struct Texture;
class Shader;
class Mesh;

struct Material {
	uint32_t flag;
	glm::vec3 color;
	float alphaCutout;
	std::vector<Texture> textures;
};

struct MaterialBatch {
	const Material* material{};
	const Shader* shader{};
	std::vector<Mesh>* meshes{};
};
