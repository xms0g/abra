#pragma once
#include "buffer.hpp"

class IndexBuffer final : public GPUBuffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	void copyToMemory(const void* data, uint32_t size) const;
};
