#include "uniformBuffer.h"

UniformBuffer::UniformBuffer(const BufferUsage usage, const int32_t size)
	: Buffer(GL_UNIFORM_BUFFER, size, usage) {
	bind();
	glBufferData(target(), size, nullptr, usage);
	unbind();
}

void UniformBuffer::copyToMemory(const void* data, const size_t offset, const int32_t size) const {
	bind();
	glBufferSubData(mTarget, static_cast<long>(offset), static_cast<long>(size), data);
}
