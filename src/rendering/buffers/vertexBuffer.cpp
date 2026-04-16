#include "vertexBuffer.h"

VertexBuffer::VertexBuffer(const BufferUsage usage)
	: Buffer(GL_ARRAY_BUFFER, usage) {
}

void VertexBuffer::setData(const void* data, const uint32_t size, const uint32_t offset) const {
	switch (mUsage) {
		case STATIC:
			glBufferData(mTarget, size, data, STATIC);
			break;
		case DYNAMIC: {
			if (data == nullptr) {
				glBufferData(mTarget, size, nullptr, DYNAMIC);
				return;
			}
			glBufferSubData(mTarget, offset, size, data);
			break;
		}
		default: break;
	}
}

IndexBuffer::IndexBuffer(const BufferUsage usage)
	: Buffer(GL_ELEMENT_ARRAY_BUFFER, usage) {
}

void IndexBuffer::setData(const void* data, const uint32_t size) const {
	glBufferData(mTarget, size, data, mUsage);
}

const std::vector<VertexAttribute>& VertexLayout::attributes() const {
	return mAttributes;
}

int32_t VertexLayout::stride() const {
	return mStride;
}

void VertexLayout::addPadding(const int32_t bytes) {
	mStride += bytes;
}

template<>
void VertexLayout::push<float>(const uint32_t index, const int32_t size, const bool normalized) {
	mAttributes.push_back({GL_FLOAT, index, size, normalized, mStride});
	mStride += size * sizeof(float);
}

template<>
void VertexLayout::push<int>(const uint32_t index, const int32_t size, const bool normalized) {
	mAttributes.push_back({GL_INT, index, size, normalized, mStride});
	mStride += size * sizeof(int);
}

template<>
void VertexLayout::pushMatrix<glm::mat4>(const uint32_t startIndex, const uint32_t divisor) {
	for (uint32_t i = 0; i < 4; ++i) {
		// A mat4 is 4 columns of vec4
		mAttributes.push_back({GL_FLOAT, startIndex + i, 4, false, mStride, divisor});
		mStride += sizeof(glm::vec4);
	}
}

template<>
void VertexLayout::pushMatrix<glm::mat3>(const uint32_t startIndex, const uint32_t divisor) {
	for (uint32_t i = 0; i < 3; ++i) {
		// A mat3 is 3 columns of vec3
		mAttributes.push_back({GL_FLOAT, startIndex + i, 3, false, mStride, divisor});
		mStride += sizeof(glm::vec3);
	}
}
