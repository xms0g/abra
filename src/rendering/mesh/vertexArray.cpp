#include "vertexArray.hpp"

#include <utility>

#include "glad/glad.h"

VertexArray::VertexArray() {
	glGenVertexArrays(1, &mID);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &mID);
}

VertexArray::VertexArray(VertexArray&& other) noexcept
	: mID(std::exchange(other.mID, 0)) {
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
	if (this != &other) {
		if (mID)
			glDeleteVertexArrays(1, &mID);
		mID = std::exchange(other.mID, 0);
	}

	return *this;
}

uint32_t VertexArray::id() const {
	return mID;
}

void VertexArray::bind() const {
	glBindVertexArray(mID);
}

void VertexArray::unbind() {
	glBindVertexArray(0);
}

void VertexArray::setAttribute(const uint32_t index,
                               const int32_t size,
                               const int32_t type,
                               const bool normalized,
                               const int32_t stride,
                               const void* offset,
                               const uint32_t divisor) {
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, offset);

	if (divisor > 0)
		glVertexAttribDivisor(index, divisor);
}
