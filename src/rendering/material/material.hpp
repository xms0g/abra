#pragma once
#include <cstdint>
#include <vector>
#include <utility>
#include "glm/glm.hpp"
#include "../texture.hpp"

enum class MaterialFlag: uint32_t {
	None,
	Opaque = 1 << 0,
	Blend = 1 << 1,
	Castshadow = 1 << 2,
	Twosided = 1 << 3,
	Unlit = 1 << 4,
	Pbr = 1 << 5,
	HasHeightMap = 1 << 6,
	HasEmissiveMap = 1 << 7,
	HasAoMap = 1 << 8,
	HasOrm = 1 << 9,
	Alphacutoff = 1 << 10,
	HasSolidColor = 1 << 11,
};

constexpr MaterialFlag operator|(const MaterialFlag lhs, const MaterialFlag rhs) {
	return static_cast<MaterialFlag>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

constexpr MaterialFlag& operator|=(MaterialFlag& lhs, const MaterialFlag rhs) {
	lhs = static_cast<MaterialFlag>(std::to_underlying(lhs) | std::to_underlying(rhs));
	return lhs;
}

constexpr MaterialFlag operator&(const MaterialFlag lhs, const MaterialFlag rhs) {
	return static_cast<MaterialFlag>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

struct Material {
	uint32_t idx{0};
	MaterialFlag flags{};
	TextureTarget textureTarget{};
	glm::vec3 color{0.0f};
	float alphaCutoff{0.0f};
	std::vector<MaterialTexture> textures;

	Material() = default;

	Material(const Material& other) = delete;

	Material& operator=(const Material& other) = delete;

	Material(Material&& other) noexcept
		: idx(std::exchange(other.idx, 0)),
		  flags(std::exchange(other.flags, {})),
		  textureTarget(std::exchange(other.textureTarget, {})),
		  color(other.color),
		  alphaCutoff(std::exchange(other.alphaCutoff, 0.0f)),
		  textures(std::move(other.textures)) {
	}

	Material& operator=(Material&& other) noexcept {
		if (this != &other) {
			idx = std::exchange(other.idx, 0);
			flags = std::exchange(other.flags, {});
			textureTarget = std::exchange(other.textureTarget, {});
			color = other.color;
			alphaCutoff = std::exchange(other.alphaCutoff, 0.0f);
			textures = std::move(other.textures);
		}
		return *this;
	}
};

struct MaterialBatch {
	uint32_t materialIndex{};
	MaterialFlag materialFlags{};
	uint32_t renderFlag{};
	uint32_t textureOffset{};
	size_t textureCount{};
	std::vector<uint32_t> meshIndices;
};
