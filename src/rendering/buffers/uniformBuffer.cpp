#include "uniformBuffer.h"

UniformBuffer::UniformBuffer(const BufferUsage usage, const int32_t size, const uint32_t binding)
	: Buffer(GL_UNIFORM_BUFFER, usage) {
	bind();
	glBufferData(mTarget, size, nullptr, usage);
	unbind();
	glBindBufferRange(mTarget, binding, mID, 0, size);
}

void UniformBuffer::setData(const void* data, const size_t size, const size_t offset) const {
	glBufferSubData(mTarget, static_cast<long>(offset), static_cast<long>(size), data);
}

void UniformBuffer::configure(
	const uint32_t program,
	const uint32_t uniformBlockBinding,
	const char* uniformBlockName) const {
	const uint32_t ubidx = glGetUniformBlockIndex(program, uniformBlockName);
	glUniformBlockBinding(program, ubidx, uniformBlockBinding);
}
