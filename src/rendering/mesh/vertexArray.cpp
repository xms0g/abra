#include "vertexArray.h"
#include "glad/glad.h"

VertexArray::VertexArray() {
	glGenVertexArrays(1, &mID);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &mID);
}

uint32_t VertexArray::id() const {
	return mID;
}

void VertexArray::bind() const {
	glBindVertexArray(mID);
}

void VertexArray::unbind() const {
	glBindVertexArray(0);
}

void VertexArray::setAttribute(
	const uint32_t index,
	const int32_t size,
	const int32_t type,
	const bool normalized,
	const int32_t stride,
	const void* offset) {
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, offset);
}
