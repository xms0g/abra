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
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
