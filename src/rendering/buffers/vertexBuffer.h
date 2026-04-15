#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
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

	[[nodiscard]] const std::vector<VertexAttribute>& getAttributes() const { return mAttributes; }

	[[nodiscard]] int32_t getStride() const { return mStride; }

	template<typename T>
	void push(uint32_t index, int32_t size, bool normalized = false) {
		// This will be specialized below
		static_assert(false, "Type not supported in VertexLayout");
	}

	// Specialization for Float
	template<>
	void push<float>(const uint32_t index, const int32_t size, const bool normalized) {
		mAttributes.push_back({GL_FLOAT, index, size, normalized, mStride});
		mStride += size * sizeof(float);
	}

	// Specialization for Int (Used for Bone IDs)
	template<>
	void push<int>(const uint32_t index, const int32_t size, const bool normalized) {
		mAttributes.push_back({GL_INT, index, size, normalized, mStride});
		mStride += size * sizeof(int);
	}

	template<typename T>
	void pushMatrix(const uint32_t startIndex, const uint32_t divisor = 1) {
		if constexpr (std::is_same_v<T, glm::mat4>) {
			for (uint32_t i = 0; i < 4; ++i) {
				// A mat4 is 4 columns of vec4
				mAttributes.push_back({GL_FLOAT, startIndex + i, 4, false, mStride, divisor});
				mStride += sizeof(glm::vec4);
			}
		} else if constexpr (std::is_same_v<T, glm::mat3>) {
			for (uint32_t i = 0; i < 3; ++i) {
				// A mat3 is 3 columns of vec3
				mAttributes.push_back({GL_FLOAT, startIndex + i, 3, false, mStride, divisor});
				mStride += sizeof(glm::vec3);
			}
		}
	}

private:
	std::vector<VertexAttribute> mAttributes;
	int32_t mStride;
};
