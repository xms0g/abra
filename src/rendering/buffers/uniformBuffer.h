#pragma once
#include "buffer.hpp"

class UniformBuffer final : public Buffer {
public:
	UniformBuffer() = default;

	UniformBuffer(BufferUsage usage, int32_t size);

	void copyToMemory(const void* data, size_t offset, int32_t size) const;
};
