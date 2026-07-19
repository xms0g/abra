#pragma once
#include <cstdlib>
#include <string>
#include <functional>
#include "buffer.hpp"

struct UniformBinding {
	std::string name;
	uint32_t binding;

	void (*configure)(uint32_t, uint32_t, const char*);
};

class UniformBuffer final : public Buffer {
public:
	UniformBuffer() = default;

	UniformBuffer(BufferUsage usage, int32_t size, uint32_t binding);

	void setData(const void* data, size_t size, size_t offset = 0) const;

	static void configure(uint32_t program, uint32_t uniformBlockBinding, const char* uniformBlockName);
};
