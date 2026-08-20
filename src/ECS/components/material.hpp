#pragma once

struct Material;
struct GPUTexture;

struct MaterialComponent {
	using MaterialMap = std::unordered_map<uint32_t, Material>;
	MaterialMap* materials;

	float heightScale;
	uint32_t renderFlag;

	MaterialComponent() = default;

	explicit MaterialComponent(MaterialMap* mat,
	                           const float h = 1.0f,
	                           const uint32_t f = 1 << 1)
		: materials(mat),
		  heightScale(h),
		  renderFlag(f) {
	}
};
