#pragma once
#include "glm/glm.hpp"
#include "material/material.hpp"

struct MaterialView {
	uint32_t idx;
	MaterialFlag flags;
	glm::vec3 color;
	float alphaCutoff;
	float heightScale;
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
