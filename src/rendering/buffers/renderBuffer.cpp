#include "renderBuffer.h"
#include "glad/glad.h"
#include "../glUtils.hpp"

RenderBuffer::RenderBuffer(const InternalFormat format, const int32_t width, const int32_t height)
	: mFormat(format) {
	glGenRenderbuffers(1, &mHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, mHandle);

	glRenderbufferStorage(GL_RENDERBUFFER, toUnderlying(format), width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::RenderBuffer(const InternalFormat format, const int32_t width, const int32_t height, const int32_t samples)
	: mFormat(format) {
	glGenRenderbuffers(1, &mHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, mHandle);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, toUnderlying(format), width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer() {
	glDeleteRenderbuffers(1, &mHandle);
}

RenderBuffer::RenderBuffer(RenderBuffer&& other) noexcept
	: mHandle(other.mHandle),
	  mFormat(other.mFormat) {
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& other) noexcept {
	if (this != &other) {
		if (mHandle != 0)
			glDeleteRenderbuffers(1, &mHandle);
		mHandle = std::exchange(other.mHandle, 0);
		mFormat = std::exchange(other.mFormat, {});
	}

	return *this;
}

uint32_t RenderBuffer::handle() const {
	return mHandle;
}

void RenderBuffer::resize(const int32_t width, const int32_t height) const {
	glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
	glRenderbufferStorage(GL_RENDERBUFFER, toUnderlying(mFormat), width, height);
}
