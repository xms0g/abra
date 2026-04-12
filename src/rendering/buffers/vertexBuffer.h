#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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

enum BufferUsage {
	STATIC = GL_STATIC_DRAW,
	DYNAMIC = GL_DYNAMIC_DRAW,
	STREAM = GL_STREAM_DRAW
};

class VertexBuffer {
public:
	explicit VertexBuffer(BufferUsage usage);

	~VertexBuffer();

	void bind() const;

	void unbind() const;

	void setData(const void* data, uint32_t size) const;

private:
	uint32_t mVBO{0};
	BufferUsage mUsage{STATIC};
};

class IndexBuffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	~IndexBuffer();

	void bind() const;

	void unbind() const;

	void setData(const void* data, uint32_t size);

private:
	uint32_t mIBO{0};
	BufferUsage mUsage{STATIC};
};
