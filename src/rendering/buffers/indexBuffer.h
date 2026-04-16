#pragma once
#include "buffer.hpp"

class IndexBuffer : public Buffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	void setData(const void* data, uint32_t size) const;
};
