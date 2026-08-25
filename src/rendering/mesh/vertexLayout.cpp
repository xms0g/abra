#include "vertexLayout.hpp"

const std::vector<VertexAttribute>& VertexLayout::attributes() const {
	return mAttributes;
}

int32_t VertexLayout::stride() const {
	return mStride;
}

void VertexLayout::addPadding(const int32_t bytes) {
	mStride += bytes;
}
