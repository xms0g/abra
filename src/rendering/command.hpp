#pragma once
#include <span>
#include "glm/glm.hpp"
#include "texture/texture.h"

struct MaterialView {
	uint32_t idx;
	uint32_t flags;
	glm::vec3 color;
	float alphaCutoff;
	float heightScale;
	std::span<const TextureView> textures;
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
	size_t entityID{};
	uint32_t debugMode{};
	MaterialView material{};
	TransformView transform{};
	MeshView mesh{};
};
