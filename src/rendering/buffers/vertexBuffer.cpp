#include "vertexBuffer.h"
#include "glad/glad.h"

VertexBuffer::VertexBuffer(const BufferUsage usage): mUsage(usage) {
	glGenBuffers(1, &mVBO);
}

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &mVBO);
}

uint32_t VertexBuffer::offset() const {
	return mOffset;
}

void VertexBuffer::offset(const uint32_t offset) {
	mOffset = offset;
}

void VertexBuffer::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}

void VertexBuffer::unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setData(const void* data, const uint32_t size) const {
	glBufferData(GL_ARRAY_BUFFER, size, data, mUsage);
}

IndexBuffer::IndexBuffer() {
	glGenBuffers(1, &mIBO);
}

IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &mIBO);
}

void IndexBuffer::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
}

void IndexBuffer::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::setData(const void* data, const uint32_t size) {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
