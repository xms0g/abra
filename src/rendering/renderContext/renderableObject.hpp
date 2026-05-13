#pragma once
#include <cstdint>

class Shader;

struct RenderableObject {
	size_t entityID;
	uint32_t materialIndex;
	uint32_t meshIndex;
	const Shader* shader;
};
