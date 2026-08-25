#pragma once
#include <cstdint>

class VertexArray {
public:
	VertexArray();

	~VertexArray();

	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;

	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& other) noexcept;

	[[nodiscard]]
	uint32_t id() const;

	void bind() const;

	static void unbind();

	static void setAttribute(uint32_t index,
	                         int32_t size,
	                         int32_t type,
	                         bool normalized,
	                         int32_t stride,
	                         const void* offset,
	                         uint32_t divisor);

private:
	uint32_t mID{};
};
