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
