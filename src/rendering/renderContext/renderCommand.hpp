#pragma once

class Mesh;
class Shader;
struct Material;
class Entity;

struct RenderCommand {
	const Entity* entity;
	const Material* material;
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
