#pragma once
#include "buffer.hpp"

class IndexBuffer final : public GPUBuffer {
public:
	explicit IndexBuffer(BufferUsage usage);

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;

	IndexBuffer(IndexBuffer&& other) noexcept = default;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept = default;

	void copyToMemory(const void* data, uint32_t size) const;
};
