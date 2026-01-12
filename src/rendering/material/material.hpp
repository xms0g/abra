#pragma once
#include <cstdint>
#include <vector>
#include "glm/glm.hpp"

enum MaterialFlag: uint32_t {
	OPAQUE = 1 << 0,
	BLEND = 1 << 1,
	CUTOUT = 1 << 2,
	CASTSHADOW = 1 << 3,
	TWOSIDED = 1 << 4,
	UNLIT = 1 << 5,
	PBR = 1 << 6
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
