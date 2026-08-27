#pragma once
#include <cstdint>

enum class RenderFlag: uint32_t {
	None = 0,
	ForwardPass = 1 << 0,
	DeferredPass = 1 << 1,
	InstancedPass = 1 << 2,
	TerrainPass = 1 << 3,
	SkyboxPass = 1 << 4
};
