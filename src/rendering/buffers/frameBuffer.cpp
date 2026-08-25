#include "frameBuffer.hpp"
#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "renderBuffer.hpp"
#include "../texture.hpp"

FrameBuffer::FrameBuffer(const int32_t width, const int32_t height)
	: mWidth(width),
	  mHeight(height) {
	glGenFramebuffers(1, &mHandle);
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
	: mHandle(std::exchange(other.mHandle, 0)),
	  mWidth(std::exchange(other.mWidth, 0)),
	  mHeight(std::exchange(other.mHeight, 0)),
	  mAttachments(std::move(other.mAttachments)) {
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	if (this != &other) {
		if (mHandle) {
			glDeleteFramebuffers(1, &mHandle);
		}

		mHandle = std::exchange(other.mHandle, 0);
		mWidth = std::exchange(other.mWidth, 0);
		mHeight = std::exchange(other.mHeight, 0);
		mAttachments = std::move(other.mAttachments);
	}
	return *this;
}

FrameBuffer::~FrameBuffer() {
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

const std::shared_ptr<GPUTexture>& FrameBuffer::texture(const uint32_t index) const {
	return mAttachments.textures[index];
}

RenderBuffer& FrameBuffer::renderBuffer(const uint32_t index) {
	return mAttachments.renderBuffers[index];
}

FrameBuffer& FrameBuffer::attachColor(std::shared_ptr<GPUTexture>& texture) {
	const GLenum attachment = std::to_underlying(Attachment::Color0) + mAttachments.colorAttachmentCount++;
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, std::to_underlying(texture->target()), texture->id(), 0);

	mAttachments.textures.emplace_back(std::move(texture));
	return *this;
}

FrameBuffer& FrameBuffer::attachDepth(std::shared_ptr<GPUTexture>& texture) {
	if (texture->target() == TextureTarget::Texture2D) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, std::to_underlying(Attachment::Depth), std::to_underlying(texture->target()), texture->id(), 0);
	} else {
		glFramebufferTexture(GL_FRAMEBUFFER, std::to_underlying(Attachment::Depth), texture->id(), 0);
	}

	mAttachments.textures.emplace_back(std::move(texture));
	return *this;
}

FrameBuffer& FrameBuffer::attachDepth(RenderBuffer& renderBuffer) {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, std::to_underlying(Attachment::Depth), GL_RENDERBUFFER, renderBuffer.handle());

	mAttachments.renderBuffers.emplace_back(std::move(renderBuffer));
	return *this;
}

FrameBuffer& FrameBuffer::attachDepthStencil(RenderBuffer& renderBuffer) {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer.handle());

	mAttachments.renderBuffers.emplace_back(std::move(renderBuffer));
	return *this;
}

FrameBuffer& FrameBuffer::configureDrawBuffers() {
	std::vector<GLenum> buffers;
	buffers.reserve(mAttachments.colorAttachmentCount);

	for (uint32_t i = 0; i < mAttachments.colorAttachmentCount; ++i)
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