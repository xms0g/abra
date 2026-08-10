#pragma once
#include "buffer.hpp"

class UniformBuffer final : public Buffer {
public:
	UniformBuffer() = default;

	UniformBuffer(BufferUsage usage, int32_t size);

	[[nodiscard]]
	int32_t size() const;

	void copyToMemory(const void* data, size_t offset = 0) const;

private:
	int32_t mSize{0};
};
