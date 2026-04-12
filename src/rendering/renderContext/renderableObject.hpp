#pragma once

struct EntityCore;
class Mesh;
class Shader;
struct Material;

struct RenderableObject {
	const EntityCore* entity;
	const Material* material;
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
