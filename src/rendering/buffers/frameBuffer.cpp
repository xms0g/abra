#include "frameBuffer.h"
#include <cassert>
#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "renderBuffer.h"
#include "../glUtils.hpp"
#include "../texture/texture.h"

FrameBuffer::FrameBuffer(const int32_t width, const int32_t height)
	: mWidth(width),
	  mHeight(height) {
	glGenFramebuffers(1, &mHandle);
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
	: mHandle(std::exchange(other.mHandle, 0)),
	  mWidth(std::exchange(other.mWidth, 0)),
	  mHeight(std::exchange(other.mHeight, 0)),
	  mTextures(std::move(other.mTextures)) {
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	if (this != &other) {
		if (mHandle) {
			glDeleteFramebuffers(1, &mHandle);
		}

		mHandle = std::exchange(other.mHandle, 0);
		mWidth = std::exchange(other.mWidth, 0);
		mHeight = std::exchange(other.mHeight, 0);
		mTextures = std::move(other.mTextures);
	}
	return *this;
}

FrameBuffer::~FrameBuffer() {
	if (!mTextures.empty()) {
		for (const auto& texture: mTextures) {
			glDeleteTextures(1, &texture.id);
		}
	}

	glDeleteFramebuffers(1, &mHandle);
}

uint32_t FrameBuffer::handle() const {
	return mHandle;
}

int32_t FrameBuffer::width() const {
	return mWidth;
}

int32_t FrameBuffer::height() const {
	return mHeight;
}

TextureView FrameBuffer::texture(const uint32_t index) const {
	return {.id = mTextures[index].id, .target = mTextures[index].target};
}

RenderBuffer& FrameBuffer::renderBuffer(const uint32_t index) {
	return mRenderBuffers[index];
}

FrameBuffer& FrameBuffer::attachColor(Texture& texture) {
	mTextures.emplace_back(std::move(texture));

	const auto& tex = mTextures.back();
	const GLenum attachment = toUnderlying(Attachment::Color0) + mColorAttachmentCount++;
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, toUnderlying(tex.target), tex.id, 0);

	return *this;
}

FrameBuffer& FrameBuffer::attachDepth(Texture& texture) {
	mTextures.emplace_back(std::move(texture));

	if (const auto& tex = mTextures.back(); tex.target == TextureTarget::Texture2D) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, toUnderlying(Attachment::Depth), toUnderlying(tex.target), tex.id, 0);
	} else {
		glFramebufferTexture(GL_FRAMEBUFFER, toUnderlying(Attachment::Depth), tex.id, 0);
	}

	return *this;
}

FrameBuffer& FrameBuffer::attachDepth(RenderBuffer& renderBuffer) {
	mRenderBuffers.emplace_back(std::move(renderBuffer));

	const auto& buffer = mRenderBuffers.back();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, toUnderlying(Attachment::Depth), GL_RENDERBUFFER, buffer.handle());
	return *this;
}

FrameBuffer& FrameBuffer::attachDepthStencil(RenderBuffer& renderBuffer) {
	mRenderBuffers.emplace_back(std::move(renderBuffer));

	const auto& buffer = mRenderBuffers.back();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.handle());
	return *this;
}

FrameBuffer& FrameBuffer::configureDrawBuffers() {
	std::vector<GLenum> buffers;
	buffers.reserve(mColorAttachmentCount);

	for (uint32_t i = 0; i < mColorAttachmentCount; ++i)
		buffers.push_back(GL_COLOR_ATTACHMENT0 + i);

	if (buffers.empty()) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	} else {
		glDrawBuffers(static_cast<int32_t>(buffers.size()), buffers.data());
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}

	return *this;
}

void FrameBuffer::checkStatus() {
	if (const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
			case GL_FRAMEBUFFER_UNDEFINED:
				throw std::runtime_error("FRAMEBUFFER_UNDEFINED\n");
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
			case GL_FRAMEBUFFER_UNSUPPORTED:
				throw std::runtime_error("FRAMEBUFFER_UNSUPPORTED\n");
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				throw std::runtime_error("FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
			default:
				break;
		}
	}
}