#pragma once
#include "../../rendering/context/renderFlags.hpp"

struct Material;
class GPUTexture;

struct MaterialComponent {
	using MaterialMap = std::unordered_map<uint32_t, Material>;
	MaterialMap* materials;

	float heightScale;
	RenderFlag renderFlag;

	MaterialComponent() = default;

	explicit MaterialComponent(MaterialMap* mat,
	                           const float h = 1.0f,
	                           const RenderFlag f = RenderFlag::DEFERRED_PASS)
		: materials(mat),
		  heightScale(h),
		  renderFlag(f) {
	}
};
