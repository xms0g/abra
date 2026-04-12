#include "vertexBuffer.h"

VertexBuffer::VertexBuffer(const BufferUsage usage) : mUsage(usage) {
	glGenBuffers(1, &mVBO);
}

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &mVBO);
}

void VertexBuffer::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}

void VertexBuffer::unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setData(const void* data, const uint32_t size, const uint32_t offset) const {
	switch (mUsage) {
		case STATIC:
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			break;
		case DYNAMIC: {
			if (data == nullptr) {
				glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
				return;
			}
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			break;
		}
		default: break;
	}
}

IndexBuffer::IndexBuffer(const BufferUsage usage) : mUsage(usage) {
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

void IndexBuffer::setData(const void* data, const uint32_t size) const {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, mUsage);
}
