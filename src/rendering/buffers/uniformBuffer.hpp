#pragma once
#include "buffer.hpp"

class UniformBuffer final : public GPUBuffer {
public:
	UniformBuffer() = default;

	UniformBuffer(int32_t size, BufferUsage usage);

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	UniformBuffer(UniformBuffer&&) noexcept = default;
	UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

	void copyToMemory(const void* data, std::size_t offset, int32_t size) const;
};
