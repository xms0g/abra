#include "uniformBuffer.h"
#include "../glUtils.hpp"

UniformBuffer::UniformBuffer(const BufferUsage usage, const int32_t size)
	: Buffer(BufferType::Uniform, size, usage) {
	bind();
	glBufferData(toUnderlying(mTarget), size, nullptr, toUnderlying(usage));
	unbind();
}

void UniformBuffer::copyToMemory(const void* data, const size_t offset, const int32_t size) const {
	bind();
	glBufferSubData(toUnderlying(mTarget), static_cast<long>(offset), static_cast<long>(size), data);
}
