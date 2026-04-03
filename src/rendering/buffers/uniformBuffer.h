#pragma once
#include <cstdlib>

class UniformBuffer {
public:
	explicit UniformBuffer(int32_t size, uint32_t binding);

	~UniformBuffer();

	void bind() const;

	void unbind() const;

	void setData(const void* data, size_t size, size_t offset = 0) const;

	void configure(uint32_t program, uint32_t uniformBlockBinding, const char* uniformBlockName) const;

private:
	uint32_t mUBO{};
};
