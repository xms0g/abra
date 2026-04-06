#pragma once
#include <cstdint>
#include <unordered_set>
#include <vector>
#include "glm/glm.hpp"
#include "../texture/texture.h"

enum MaterialFlag: uint32_t {
	OPAQUE = 1 << 0,
	BLEND = 1 << 1,
	CASTSHADOW = 1 << 2,
	TWOSIDED = 1 << 3,
	UNLIT = 1 << 4,
	PBR = 1 << 5
};

class Shader;
class Mesh;

struct Material {
	uint32_t flag;
	glm::vec3 color;
	float alphaCutoff;
	std::vector<Texture> textures;

	[[nodiscard]] bool hasTexture(const std::string_view p, uint32_t desiredType) const {
		return std::find_if(textures.begin(), textures.end(),
		                    [p, desiredType](const Texture& tex) {
			                    return tex.path == p && tex.type == desiredType;
		                    }) != textures.end();
	}
};

struct MaterialBatch {
	const Material* material{};
	const Shader* shader{};
	std::vector<Mesh>* meshes{};
};
