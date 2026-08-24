#include "uniformBuffer.hpp"

UniformBuffer::UniformBuffer(const int32_t size, const BufferUsage usage)
	: GPUBuffer(BufferType::Uniform, size, usage) {
	bind();
	glBufferData(std::to_underlying(mTarget), size, nullptr, std::to_underlying(usage));
	unbind();
}

void UniformBuffer::copyToMemory(const void* data, const std::size_t offset, const int32_t size) const {
	bind();
	glBufferSubData(std::to_underlying(mTarget), static_cast<long>(offset), static_cast<long>(size), data);
}
