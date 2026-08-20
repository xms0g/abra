#include "indexBuffer.h"

IndexBuffer::IndexBuffer(const BufferUsage usage)
	: Buffer(GL_ELEMENT_ARRAY_BUFFER, 0, usage) {
}

void IndexBuffer::copyToMemory(const void* data, const uint32_t size) const {
	bind();
	glBufferData(mTarget, size, data, mUsage);
}
