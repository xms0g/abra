#include "frameBuffer.h"
#include <cassert>
#include <vector>
#include "glad/glad.h"

BaseFrameBuffer::BaseFrameBuffer(const int32_t width, const int32_t height)
	: mWidth(width),
	  mHeight(height) {
	glGenFramebuffers(1, &mFBO);
	bind();
}

BaseFrameBuffer::~BaseFrameBuffer() {
	glDeleteFramebuffers(1, &mFBO);

	if (mRBO)
		glDeleteRenderbuffers(1, &mRBO);
}

int32_t BaseFrameBuffer::width() const {
	return mWidth;
}

int32_t BaseFrameBuffer::height() const {
	return mHeight;
}

uint32_t BaseFrameBuffer::rbo() const {
	return mRBO;
}

void BaseFrameBuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, mWidth, mHeight);
}

void BaseFrameBuffer::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BaseFrameBuffer::checkStatus() {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR::FRAMEBUFFER::NOT_COMPLETE!\n");
	}
}

FrameBuffer::FrameBuffer(const int32_t width, const int32_t height)
	: BaseFrameBuffer(width, height) {
}

FrameBuffer::~FrameBuffer() {
	if (!mTextures.empty()) {
		for (const auto& [textureID, type]: mTextures) {
			glDeleteTextures(1, &textureID);
		}
	}
}

const std::vector<std::pair<uint32_t, uint32_t> >& FrameBuffer::textures() const {
	return mTextures;
}

void FrameBuffer::bindForRead() const {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
}

void FrameBuffer::bindForDraw() const {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
}

FrameBuffer& FrameBuffer::withTexture(const uint32_t format) {
	const int32_t internalFormat = getInternalFormat(format);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		mWidth,
		mHeight,
		0,
		format,
		GL_UNSIGNED_BYTE,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAttachment(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureMultisampled(const int32_t multisampledCount, const uint32_t format) {
	const int32_t internalFormat = getInternalFormat(format);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		multisampledCount,
		internalFormat,
		mWidth,
		mHeight,
		GL_TRUE);

	setAttachment(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureFP(const uint32_t format) {
	const int32_t internalFormat = getInternalFormat(format, true);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		mWidth,
		mHeight,
		0,
		format,
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

FrameBuffer& FrameBuffer::withTextureFPMultisampled(
	const int32_t multisampledCount,
	const uint32_t format) {
	const int32_t internalFormat = getInternalFormat(format, true);

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		multisampledCount,
		internalFormat,
		mWidth,
		mHeight,
		GL_TRUE);

	setAttachment(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureDepth(const int32_t internalFormat, const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
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
	const int32_t internalFormat,
	const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_2D_ARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		internalFormat,
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

FrameBuffer& FrameBuffer::withTextureCubemapDepth(const int32_t internalFormat, const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (uint32_t i = 0; i < 6; ++i)
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			internalFormat,
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
	const int32_t internalFormat,
	const bool onlyForShadowMap) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextures.emplace_back(textureID, GL_TEXTURE_CUBE_MAP_ARRAY);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textureID);
	glTexImage3D(
		GL_TEXTURE_CUBE_MAP_ARRAY,
		0,
		internalFormat,
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

FrameBuffer& FrameBuffer::withRenderBufferDepth(const uint32_t internalFormat) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthMultisampled(const int32_t multisampledCount,
                                                            const uint32_t internalFormat) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, internalFormat, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencil(const int32_t internalFormat) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencilMultisampled(const int32_t multisampledCount,
                                                                   const int32_t internalFormat) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, internalFormat, mWidth, mHeight);
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

uint32_t FrameBuffer::textureImpl(const uint32_t index) const {
	return mTextures[index].first;
}

void FrameBuffer::bindTextureImpl(uint32_t slot, uint32_t textureIndex) const {
	auto [id, type] = mTextures[textureIndex];

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(type, id);
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

int32_t FrameBuffer::getInternalFormat(const uint32_t format, const bool isFloat) {
	if (isFloat) {
		switch (format) {
			case GL_RED: return GL_R16F;
			case GL_RG: return GL_RG16F;
			case GL_RGB: return GL_RGB16F;
			default: return GL_RGBA16F;
		}
	}

	switch (format) {
		case GL_RED: return GL_R8;
		case GL_RG: return GL_RG8;
		case GL_RGB: return GL_RGB8;
		default: return GL_RGBA8;
	}

	assert(false && "Unsupported format");

	return 0;
}

CubemapBuffer::CubemapBuffer(const int32_t width, const int32_t height, const bool mipmap, const bool prefilter)
	: BaseFrameBuffer(width, height) {
	glGenRenderbuffers(1, &mRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);

	glGenTextures(1, &mCubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapID);

	for (uint32_t i = 0; i < 6; ++i) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			static_cast<int32_t>(width),
			static_cast<int32_t>(height),
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (prefilter) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CubemapBuffer::~CubemapBuffer() {
	glDeleteTextures(1, &mCubemapID);
}

void CubemapBuffer::bindFace(const uint32_t face, const int32_t mip) const {
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
		mCubemapID,
		mip);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

uint32_t CubemapBuffer::textureImpl(uint32_t index) const {
	return mCubemapID;
}

void CubemapBuffer::bindTextureImpl(const uint32_t slot, uint32_t textureIndex) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapID);
}
