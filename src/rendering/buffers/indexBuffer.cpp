#include "indexBuffer.h"
#include "../glUtils.hpp"

IndexBuffer::IndexBuffer(const BufferUsage usage)
	: Buffer(BufferType::Index, 0, usage) {
}

void IndexBuffer::copyToMemory(const void* data, const uint32_t size) const {
	bind();
	glBufferData(toUnderlying(mTarget), size, data, toUnderlying(mUsage));
}
