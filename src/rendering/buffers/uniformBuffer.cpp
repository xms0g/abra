#include "uniformBuffer.h"
#include "glad/glad.h"

UniformBuffer::UniformBuffer(const int32_t size, const uint32_t binding) {
	glGenBuffers(1, &mUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, mUBO, 0, size);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &mUBO);
}

void UniformBuffer::bind() const {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
}

void UniformBuffer::unbind() const {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::setData(const void* data, const size_t size, const size_t offset) const {
	glBufferSubData(GL_UNIFORM_BUFFER, static_cast<long>(offset), static_cast<long>(size), data);
}

void UniformBuffer::configure(
	const uint32_t program,
	const uint32_t uniformBlockBinding,
	const char* uniformBlockName) const {
	const uint32_t ubidx = glGetUniformBlockIndex(program, uniformBlockName);
	glUniformBlockBinding(program, ubidx, uniformBlockBinding);
}
