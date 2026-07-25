#pragma once
#include <span>
#include "glm/glm.hpp"

struct MaterialView {
	uint32_t idx;
	uint32_t flags;
	uint32_t textureTarget;
	glm::vec3 color;
	float alphaCutoff;
	float heightScale;
	std::span<const uint32_t> textures;
};

struct TransformView {
	glm::mat4 model;
	glm::mat3 normal;
};

struct MeshView {
	uint32_t vao;
	size_t vertexCount;
	size_t indexCount;
};

struct DrawCommand {
	MaterialView material{};
	TransformView transform{};
	MeshView mesh{};
};
