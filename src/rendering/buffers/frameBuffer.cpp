#include "frameBuffer.h"
#include <cassert>
#include <vector>
#include "glad/glad.h"

GL(Attachment)
GL(BaseFormat)
GL(InternalFormat)

FrameBuffer::FrameBuffer(const int32_t width, const int32_t height)
	: mWidth(width),
	  mHeight(height) {
	glGenFramebuffers(1, &mFBO);
	bind();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
	: mFBO(other.mFBO),
	  mRBO(other.mRBO),
	  mWidth(other.mWidth),
	  mHeight(other.mHeight),
	  mTextures(std::move(other.mTextures)),
	  mAttachments(std::move(other.mAttachments)) {
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	if (this == &other) {
		return *this;
	}

	if (mFBO) {
		glDeleteFramebuffers(1, &mFBO);
	}
	if (mRBO) {
		glDeleteRenderbuffers(1, &mRBO);
	}

	mFBO = std::exchange(other.mFBO, 0);
	mRBO = std::exchange(other.mRBO, 0);
	mWidth = std::exchange(other.mWidth, 0);
	mHeight = std::exchange(other.mHeight, 0);
	mTextures = std::move(other.mTextures);
	mAttachments = std::move(other.mAttachments);
	return *this;
}

FrameBuffer::~FrameBuffer() {
	if (!mTextures.empty()) {
		for (const auto& [id, target]: mTextures) {
			glDeleteTextures(1, &id);
		}
	}

	glDeleteFramebuffers(1, &mFBO);

	if (mRBO)
		glDeleteRenderbuffers(1, &mRBO);
}

int32_t FrameBuffer::width() const {
	return mWidth;
}

int32_t FrameBuffer::height() const {
	return mHeight;
}

TextureView FrameBuffer::texture(uint32_t index) const {
	return {.id = mTextures[index].id, .target = mTextures[index].target};
}

void FrameBuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
}

void FrameBuffer::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resizeRenderBuffer(int32_t width, int32_t height) const {
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glViewport(0, 0, width, height);
}

void FrameBuffer::attachTexture(const uint32_t index, const Attachment attachment, const int32_t mip,
                                const int32_t layer) const {
	switch (auto [id, target] = texture(index); target) {
		case TextureTarget::Texture2D:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toGL(attachment),
				GL_TEXTURE_2D,
				id,
				mip);
			break;
		case TextureTarget::TextureCubeMap:
			assert(layer >= 0 && layer < 6);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				toGL(attachment),
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
				id,
				mip);
			break;
		case TextureTarget::Texture2DArray:
		case TextureTarget::TextureCubeMapArray:
			glFramebufferTextureLayer(
				GL_FRAMEBUFFER,
				toGL(attachment),
				id,
				mip,
				layer);
			break;
		case TextureTarget::Texture2DMultisample:
			break;
	}
}

void FrameBuffer::checkStatus() {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR::FRAMEBUFFER::NOT_COMPLETE!\n");
	}
}

void FrameBuffer::bindForRead() const {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
}

void FrameBuffer::bindForDraw() const {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
}

FrameBuffer& FrameBuffer::withTexture(const BaseFormat format) {
	const InternalFormat internalFormat = getInternalFormat(format);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		toGL(internalFormat),
		mWidth,
		mHeight,
		0,
		toGL(format),
		GL_UNSIGNED_BYTE,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAttachment(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureMultisampled(const int32_t multisampledCount, const BaseFormat format) {
	const InternalFormat internalFormat = getInternalFormat(format);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2DMultisample);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		multisampledCount,
		toGL(internalFormat),
		mWidth,
		mHeight,
		GL_TRUE);

	setAttachment(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureFP(const BaseFormat format) {
	const InternalFormat internalFormat = getInternalFormat(format, true);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		toGL(internalFormat),
		mWidth,
		mHeight,
		0,
		toGL(format),
		GL_FLOAT,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAttachment(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureFPMultisampled(const int32_t multisampledCount, const BaseFormat format) {
	const InternalFormat internalFormat = getInternalFormat(format, true);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2DMultisample);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		multisampledCount,
		toGL(internalFormat),
		mWidth,
		mHeight,
		GL_TRUE);

	setAttachment(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureDepth(const InternalFormat format, const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		toGL(format),
		mWidth,
		mHeight,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);

	setDepthTextureParameters(GL_TEXTURE_2D, 2);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

	if (onlyForShadowMap) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureDepthArray(
	const int32_t layerCount,
	const InternalFormat format,
	const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::Texture2DArray);

	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		toGL(format),
		mWidth,
		mHeight,
		layerCount,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);

	setDepthTextureParameters(GL_TEXTURE_2D_ARRAY, 2);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0);

	if (onlyForShadowMap) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	return *this;
}

FrameBuffer& FrameBuffer::withTextureCubeMap() {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::TextureCubeMap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (uint32_t i = 0; i < 6; ++i) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			static_cast<int32_t>(mWidth),
			static_cast<int32_t>(mHeight),
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureCubemapDepth(const InternalFormat format, const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::TextureCubeMap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (uint32_t i = 0; i < 6; ++i)
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			toGL(format),
			mWidth,
			mHeight,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr);
	setDepthTextureParameters(GL_TEXTURE_CUBE_MAP, 3);
	// attach depth texture as FBO's depth buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0);

	if (onlyForShadowMap) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureCubemapDepthArray(
	const int32_t layerCount,
	const InternalFormat format,
	const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, TextureTarget::TextureCubeMapArray);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textureID);
	glTexImage3D(
		GL_TEXTURE_CUBE_MAP_ARRAY,
		0,
		toGL(format),
		mWidth,
		mHeight,
		layerCount * 6,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);

	setDepthTextureParameters(GL_TEXTURE_CUBE_MAP_ARRAY, 3);
	// attach depth texture as FBO's depth buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0);

	if (onlyForShadowMap) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepth(const InternalFormat format) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, toGL(format), mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthMultisampled(
	const int32_t multisampledCount,
	const InternalFormat format) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, toGL(format), mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencil(const InternalFormat format) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, toGL(format), mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencilMultisampled(
	const int32_t multisampledCount,
	const InternalFormat format) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, toGL(format), mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::configureAttachments() {
	if (mTextures.empty()) {
		throw std::runtime_error("ERROR::FRAMEBUFFER::NO_TEXTURES_ATTACHED!\n");
	}

	glDrawBuffers(static_cast<int32_t>(mAttachments.size()), mAttachments.data());

	return *this;
}

void FrameBuffer::setAttachment(const uint32_t textureID, const uint32_t target) {
	const uint32_t attachment = GL_COLOR_ATTACHMENT0 + mTextures.size() - 1;
	mAttachments.push_back(attachment);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, textureID, 0);
}

void FrameBuffer::setDepthTextureParameters(const uint32_t target, const int32_t dim) {
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (dim == 3) {
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	}

	constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
}

InternalFormat FrameBuffer::getInternalFormat(const BaseFormat format, const bool isFloat) {
	if (isFloat) {
		switch (format) {
			case BaseFormat::Red: return InternalFormat::RedFloat;
			case BaseFormat::RG: return InternalFormat::RGFloat;
			case BaseFormat::RGB: return InternalFormat::RGBFloat;
			default: return InternalFormat::RGBAFloat;
		}
	}

	switch (format) {
		case BaseFormat::Red: return InternalFormat::Red;
		case BaseFormat::RG: return InternalFormat::RG;
		case BaseFormat::RGB: return InternalFormat::RGB;
		default: return InternalFormat::RGBA;
	}

	assert(false && "Unsupported format");
}
