#pragma once
#include <vector>
#include "buffer.hpp"

class VertexBuffer final : public Buffer {
public:
	explicit VertexBuffer(int32_t size, BufferUsage usage);

	void copyToMemory(const void* data, uint32_t offset, uint32_t size) const;
};

struct VertexAttribute {
	int32_t type;
	uint32_t index;
	int32_t size;
	bool normalized;
	int32_t offset;
	uint32_t divisor;
};

// Vertex Layout
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

template<typename T>
void VertexLayout::push(const uint32_t index, const int32_t size, const bool normalized) {
	mAttributes.push_back({
		.type = std::is_same_v<T, float> ? GL_FLOAT : GL_INT,
		.index = index,
		.size = size,
		.normalized = normalized,
		.offset = mStride,
		.divisor = 0
	});

	mStride += size * sizeof(T);
}

template<typename T>
void VertexLayout::pushVector(const uint32_t index, const bool normalized) {
	mAttributes.push_back({
		.type = std::is_same_v<typename T::value_type, float> ? GL_FLOAT : GL_INT,
		.index = index,
		.size = T::length(),
		.normalized = normalized,
		.offset = mStride,
		.divisor = 0
	});

	mStride += sizeof(T);
}

template<typename T>
void VertexLayout::pushMatrix(const uint32_t index, const uint32_t divisor) {
	using ColumnType = T::col_type;

	constexpr uint32_t numCols = T::length();
	constexpr int componentsPerCol = ColumnType::length();

	for (uint32_t i = 0; i < numCols; ++i) {
		mAttributes.push_back({
			.type = std::is_same_v<typename T::value_type, float> ? GL_FLOAT : GL_INT,
			.index = index + i,
			.size = componentsPerCol,
			.normalized = false,
			.offset = mStride,
			.divisor = divisor
		});
		mStride += sizeof(ColumnType);
	}
}
