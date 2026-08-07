#include "uniformBuffer.h"

UniformBuffer::UniformBuffer(const BufferUsage usage, const int32_t size, const int32_t binding)
	: Buffer(GL_UNIFORM_BUFFER, usage) {
	bind();
	glBufferData(mTarget, size, nullptr, usage);
	unbind();
	glBindBufferRange(mTarget, binding, mID, 0, size);
}

void UniformBuffer::setData(const void* data, const size_t size, const size_t offset) const {
	glBufferSubData(mTarget, static_cast<long>(offset), static_cast<long>(size), data);
}
