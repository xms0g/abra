#pragma once
#include "buffer.hpp"

class UniformBuffer final : public Buffer {
public:
	UniformBuffer() = default;

	UniformBuffer(BufferUsage usage, int32_t size, int32_t binding);

	void setData(const void* data, size_t size, size_t offset = 0) const;
};
