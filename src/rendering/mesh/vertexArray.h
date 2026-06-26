#pragma once
#include <cstdint>

class VertexArray {
public:
	VertexArray();

	~VertexArray();

	[[nodiscard]]
	uint32_t id() const;

	void bind() const;

	static void unbind();

	static void setAttribute(
		uint32_t index,
		int32_t size,
		int32_t type,
		bool normalized,
		int32_t stride,
		const void* offset);

private:
	uint32_t mID{};
};
