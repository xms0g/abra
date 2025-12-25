#pragma once
#include <cstdint>

enum RenderFlags: uint32_t {
	FORWARD_PASS = 1 << 0,
	DEFERRED_PASS = 1 << 1,
	INSTANCED_PASS = 1 << 2
};
