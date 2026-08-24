#include "indexBuffer.hpp"

IndexBuffer::IndexBuffer(const BufferUsage usage)
	: GPUBuffer(BufferType::Index, 0, usage) {
}

void IndexBuffer::copyToMemory(const void* data, const uint32_t size) const {
	bind();
	glBufferData(std::to_underlying(mTarget), size, data, std::to_underlying(mUsage));
}
