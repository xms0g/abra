#pragma once
#include <cstdlib>
#include <string>
#include <functional>
#include "buffer.hpp"

class UniformBuffer;
struct UniformBinding {
	std::string name;
	uint32_t binding;
	const UniformBuffer* buffer;
	void (UniformBuffer::* configure)(uint32_t, uint32_t, const char*) const;
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer(BufferUsage usage, int32_t size, uint32_t binding);

	void setData(const void* data, size_t size, size_t offset = 0) const;

	void configure(uint32_t program, uint32_t uniformBlockBinding, const char* uniformBlockName) const;
};
