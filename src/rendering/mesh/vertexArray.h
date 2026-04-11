#pragma once
#include <cstdint>

class VertexArray {
public:
	VertexArray();

	~VertexArray();

	void bind() const;

	void unbind() const;

	void setAttribute(
		uint32_t index,
		int32_t size,
		int32_t type,
		bool normalized,
		int32_t stride,
		const void* offset);

private:
	uint32_t mVAO;
};
