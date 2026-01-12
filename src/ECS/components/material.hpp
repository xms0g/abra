#pragma once

struct Material;
struct Texture;

struct MaterialComponent {
	using MaterialMap = std::unordered_map<uint32_t, Material>;
	const MaterialMap* materials;

	float shininess;
	float heightScale;
	uint32_t renderFlag;

	MaterialComponent() = default;

	explicit MaterialComponent(const MaterialMap* mat,
	                           const float s = 32.0f,
	                           const float h = 1.0f,
	                           const uint32_t f = 1 << 1)
		: materials(mat), shininess(s), heightScale(h), renderFlag(f) {
	}
};
