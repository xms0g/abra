#pragma once
#include <vector>
#include "buffer.hpp"

class VertexBuffer final : public GPUBuffer {
public:
	explicit VertexBuffer(int32_t size, BufferUsage usage);

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	VertexBuffer(VertexBuffer&&) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

	void copyToMemory(const void* data, uint32_t offset, uint32_t size) const;
};