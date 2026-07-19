#pragma once
#include <cstdint>
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
};

class Shader;
class Mesh;

struct Material {
	uint32_t idx{0};
	uint32_t flags{0};
	uint32_t textureTarget{0};
	glm::vec3 color{0.0f};
	float alphaCutoff{0.0f};
	const Shader* shader{nullptr};
	std::vector<Texture> textures;

	Material() = default;

	Material(const Material& other) = delete;
	Material& operator=(const Material& other) = delete;

	Material(Material&& other) noexcept
		: idx(std::exchange(other.idx, 0)),
		  flags(std::exchange(other.flags, 0)),
		  textureTarget(std::exchange(other.textureTarget, 0)),
		  color(std::move(other.color)),
		  alphaCutoff(std::exchange(other.alphaCutoff, 0.0f)),
		  shader(std::exchange(other.shader, nullptr)),
		  textures(std::move(other.textures)) {
	}

	Material& operator=(Material&& other) noexcept {
		if (this == &other)
			return *this;
		idx = std::exchange(other.idx, 0);
		flags = std::exchange(other.flags, 0);
		textureTarget = std::exchange(other.textureTarget, 0);
		color = std::move(other.color);
		alphaCutoff = std::exchange(other.alphaCutoff, 0.0f);
		shader = std::exchange(other.shader, nullptr);
		textures = std::move(other.textures);
		return *this;
	}
};

struct MaterialBatch {
	uint32_t materialIndex{};
	uint32_t materialFlags{};
	uint32_t renderFlag{};
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
