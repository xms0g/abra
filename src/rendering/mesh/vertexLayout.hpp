#pragma once
#include <cstdint>
#include <vector>

struct VertexAttribute {
	int32_t type;
	uint32_t index;
	int32_t size;
	bool normalized;
	int32_t offset;
	uint32_t divisor;
};

class VertexLayout {
public:
	VertexLayout() = default;

	[[nodiscard]]
	const std::vector<VertexAttribute>& attributes() const;

	[[nodiscard]]
	int32_t stride() const;

	void addPadding(int32_t bytes);

	template<typename T>
	void push(uint32_t index, int32_t size, bool normalized = false);

	template<typename T>
	void pushVector(uint32_t index, bool normalized = false);

	template<typename T>
	void pushMatrix(uint32_t index, uint32_t divisor = 1);

private:
	std::vector<VertexAttribute> mAttributes;
	int32_t mStride{0};
};

#include "vertexLayout.tpp"