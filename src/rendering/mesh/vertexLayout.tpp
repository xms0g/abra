#pragma once
#include "glad/glad.h"

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
