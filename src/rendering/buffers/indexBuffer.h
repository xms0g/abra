#pragma once
#include "buffer.hpp"

class IndexBuffer final : public Buffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	void copyToMemory(const void* data, uint32_t size) const;
};
