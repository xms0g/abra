#include "vertexBuffer.h"
#include "../glUtils.hpp"

VertexBuffer::VertexBuffer(const int32_t size, const BufferUsage usage)
	: Buffer(BufferType::Vertex, size, usage) {
	if (size > 0) {
		bind();
		glBufferData(toUnderlying(mTarget), size, nullptr, toUnderlying(mUsage));
		unbind();
	}
}

void VertexBuffer::copyToMemory(const void* data, const uint32_t offset, const uint32_t size) const {
	bind();

	switch (mUsage) {
		case BufferUsage::Static:
			glBufferData(toUnderlying(mTarget), size, data, toUnderlying(mUsage));
			break;
		case BufferUsage::Dynamic: {
			glBufferSubData(toUnderlying(mTarget), offset, size, data);
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
