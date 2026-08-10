#include "uniformBuffer.h"

UniformBuffer::UniformBuffer(const BufferUsage usage, const int32_t size)
	: Buffer(GL_UNIFORM_BUFFER, usage),
	  mSize(size) {
	bind();
	glBufferData(mTarget, mSize, nullptr, usage);
	unbind();
}

int32_t UniformBuffer::size() const {
	return mSize;
}

void UniformBuffer::setData(const void* data, const size_t offset) const {
	glBufferSubData(mTarget, static_cast<long>(offset), static_cast<long>(mSize), data);
}
