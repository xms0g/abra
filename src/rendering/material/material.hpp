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
	PBR = 1 << 5,
	HAS_HEIGHT_MAP = 1 << 6,
	HAS_EMISSIVE_MAP = 1 << 7,
	HAS_AO_MAP = 1 << 8,
	HAS_ORM = 1 << 9,
};

class Shader;
class Mesh;

struct Material {
	uint32_t id;
	uint32_t flags;
	glm::vec3 color{0.0f};
	float alphaCutoff;
	std::vector<Texture> textures;

	[[nodiscard]]
	bool hasTexture(const std::string_view p, uint32_t desiredType) const {
		return std::ranges::any_of(textures, [p, desiredType](const Texture& tex) {
			return tex.path == p && tex.type == desiredType;
		});
	}
};

struct MaterialBatch {
	uint32_t index{};
	const Shader* shader{};
	std::vector<uint32_t> meshIndices;
};
