#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "buffer.hpp"

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

struct VertexAttribute {
	int32_t type;
	uint32_t index;
	int32_t size;
	bool normalized;
	int32_t offset;
	uint32_t divisor;
};

// Vertex Layout
class VertexLayout {
public:
	VertexLayout() : mStride(0) {
	}

	[[nodiscard]] const std::vector<VertexAttribute>& attributes() const;

	[[nodiscard]] int32_t stride() const;

	void addPadding(int32_t bytes);

	template<typename T>
	void push(uint32_t index, int32_t size, bool normalized = false) {
		// This will be specialized below
		static_assert(false, "Type not supported in VertexLayout");
	}

	template<typename T>
	void pushMatrix(const uint32_t startIndex, const uint32_t divisor = 1) {
		static_assert(false, "Type not supported in VertexLayout");
	}

	// Specialization for Float
	template<>
	void push<float>(uint32_t index, int32_t size, bool normalized);

	// Specialization for Int (Used for Bone IDs)
	template<>
	void push<int>(uint32_t index, int32_t size, bool normalized);

	template<>
	void pushMatrix<glm::mat4>(uint32_t startIndex, uint32_t divisor);

	template<>
	void pushMatrix<glm::mat3>(uint32_t startIndex, uint32_t divisor);

private:
	std::vector<VertexAttribute> mAttributes;
	int32_t mStride;
};
