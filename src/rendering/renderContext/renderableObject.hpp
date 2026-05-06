#pragma once

struct EntityCore;
class Mesh;
class Shader;
struct Material;

struct RenderableObject {
	size_t entityID;
	glm::mat4 model;
	glm::mat3 normal;
	const Material* material;
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
