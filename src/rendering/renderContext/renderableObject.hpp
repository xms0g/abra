#pragma once

struct EntityCore;
class Mesh;
class Shader;
struct Material;

struct RenderableObject {
	size_t entityID;
	uint32_t materialIndex;
	uint32_t meshIndex;
	const Shader* shader;
};
