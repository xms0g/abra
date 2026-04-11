#include "vertexArray.h"
#include "glad/glad.h"
#include "glm/glm.hpp"

VertexArray::VertexArray() {
	glGenVertexArrays(1, &mVAO);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &mVAO);
}

void VertexArray::bind() const {
	glBindVertexArray(mVAO);
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
