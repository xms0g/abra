#pragma once
#include <cstdint>

enum DebugMode: uint32_t {
	None,
	Normals,
	Wireframe
};

struct DebugComponent {
	DebugMode mode{None};

	DebugComponent() = default;
};
