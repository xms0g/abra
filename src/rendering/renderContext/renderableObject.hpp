#pragma once

struct EntityCore;
class Mesh;
class Shader;
struct Material;

struct RenderableObject {
	size_t entityID;
	const Material* material;
	const Shader* shader;
	const Mesh* mesh; // Pointer to the ONE specific visible mesh
};
