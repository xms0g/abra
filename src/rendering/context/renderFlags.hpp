#pragma once
#include <cstdint>

enum RenderFlags: uint32_t {
	FORWARD_PASS = 1 << 0,
	DEFERRED_PASS = 1 << 1,
	INSTANCED_PASS = 1 << 2,
	TERRAIN_PASS = 1 << 3,
	SKYBOX_PASS = 1 << 4
};
