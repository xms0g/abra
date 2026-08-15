#include "vertexBuffer.h"

VertexBuffer::VertexBuffer(const BufferUsage usage, const uint32_t size)
	: Buffer(GL_ARRAY_BUFFER, usage) {
	if (size > 0) {
		bind();
		glBufferData(mTarget, size, nullptr, mUsage);
		unbind();
	}
}

void VertexBuffer::copyToMemory(const void* data, const uint32_t size, const uint32_t offset) const {
	switch (mUsage) {
		case STATIC:
			glBufferData(mTarget, size, data, mUsage);
			break;
		case DYNAMIC: {
			glBufferSubData(mTarget, offset, size, data);
			break;
		}
		default: break;
	}
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
