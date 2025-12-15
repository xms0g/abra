#pragma once

struct EntityData;
class Mesh;
class Shader;
struct Material;

struct RenderCommand {
	const EntityData* entity;
	const Material* material;
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
