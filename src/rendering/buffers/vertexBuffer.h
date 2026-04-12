#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "buffer.hpp"

#define PTR(a) reinterpret_cast<void*>(offsetof(Vertex, a))

struct Vertex {
#define MAX_BONE_INFLUENCE 4
	// position
	glm::vec3 position;
	// normal
	glm::vec3 normal;
	// texCoords
	glm::vec2 texcoord;
	// tangent
	glm::vec3 tangent;
	// bitangent
	glm::vec3 bitangent;
	//bone indexes which will influence this vertex
	int32_t boneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float weights[MAX_BONE_INFLUENCE];
};

class VertexBuffer : public Buffer {
public:
	explicit VertexBuffer(BufferUsage usage);

	void setData(const void* data, uint32_t size, uint32_t offset) const;
};

class IndexBuffer : public Buffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	void setData(const void* data, uint32_t size) const;

};
