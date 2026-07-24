#pragma once
#include <cstdint>

class Shader;

struct VisibleObject {
	size_t entityID;
	uint32_t materialIndex;
	uint32_t textureOffset;
	size_t textureCount;
	uint32_t meshIndex;
	const Shader* shader;
};
