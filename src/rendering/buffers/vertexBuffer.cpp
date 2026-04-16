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

const std::vector<VertexAttribute>& VertexLayout::attributes() const {
	return mAttributes;
}

int32_t VertexLayout::stride() const {
	return mStride;
}

void VertexLayout::addPadding(const int32_t bytes) {
	mStride += bytes;
}
