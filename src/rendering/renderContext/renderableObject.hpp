#pragma once

struct EntityCore;
class Mesh;
class Shader;
struct Material;

struct RenderableObject {
	size_t entityID;
	glm::mat4 model;
	glm::mat3 normal;
	uint32_t materialIndex;
	uint32_t meshIndex;
	const Shader* shader;
};
