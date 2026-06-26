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
	ALPHACUTOFF = 1 << 10,
	HAS_SOLID_COLOR = 1 << 11,
	CUBEMAP = 1 << 12,
	TERRAIN = 1 << 13
};

class Shader;
class Mesh;

struct Material {
	uint32_t id{0};
	uint32_t idx{0};
	uint32_t flags{0};
	glm::vec3 color{0.0f};
	float alphaCutoff{0.0f};
	std::vector<Texture> textures;
};

struct MaterialBatch {
	uint32_t materialIndex{};
	uint32_t textureOffset{};
	size_t textureCount{};
	const Shader* shader{};
	std::vector<uint32_t> meshIndices;
};

struct MaterialCache {
	uint32_t lastMaterialIdx{std::numeric_limits<uint32_t>::max()};
	uint32_t lastMatFlags{0};
	const Shader* lastShader{nullptr};

	void reset() {
		lastMaterialIdx = std::numeric_limits<uint32_t>::max();
		lastMatFlags = 0;
		lastShader = nullptr;
	}
};
