#include "frameBuffer.h"
#include <vector>
#include "glad/glad.h"

FrameBuffer::FrameBuffer(const int width, const int height) : mWidth(width), mHeight(height) {
	glGenFramebuffers(1, &mFBO);
	bind();
}

FrameBuffer::~FrameBuffer() {
	if (!mTextureIDs.empty()) {
		for (const auto& textureID: mTextureIDs) {
			glDeleteTextures(1, &textureID);
		}
	}

	if (mRBO)
		glDeleteRenderbuffers(1, &mRBO);

	glDeleteFramebuffers(1, &mFBO);
}

int FrameBuffer::width() const {
	return mWidth;
}

int FrameBuffer::height() const {
	return mHeight;
}

uint32_t FrameBuffer::texture() const {
	return mTextureIDs.front();
}

const std::vector<uint32_t>& FrameBuffer::textures() const {
	return mTextureIDs;
}

void FrameBuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, mWidth, mHeight);
}

void FrameBuffer::bindForRead() const {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
}

void FrameBuffer::bindForDraw() const {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
}

void FrameBuffer::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attachLayer(const uint32_t attachment, const int layer) const {
	glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture(), 0, layer);
}

FrameBuffer& FrameBuffer::withTexture() {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		mWidth,
		mHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAttachment(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureMultisampled(const int multisampledCount) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		multisampledCount,
		GL_RGBA,
		mWidth,
		mHeight,
		GL_TRUE);

	setAttachment(textureID, GL_TEXTURE_2D_MULTISAMPLE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureFP(const uint32_t format, const bool alpha) {
	const uint32_t fmt = alpha ? GL_RGBA : GL_RGB;
	int internalFormat{0};

	if (format == 16) {
		if (alpha) {
			internalFormat = GL_RGBA16F;
		} else {
			internalFormat = GL_RGB16F;
		}
	} else if (format == 32) {
		if (alpha) {
			internalFormat = GL_RGBA32F;
		} else {
			internalFormat = GL_RGB32F;
		}
	}
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		mWidth,
		mHeight,
		0,
		fmt,
		GL_FLOAT,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setAttachment(textureID, GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withTextureFPMultisampled(
	const int multisampledCount,
	const uint32_t format,
	const bool alpha) {
	int internalFormat{0};

	if (format == 16) {
		if (alpha) {
			internalFormat = GL_RGBA16F;
		} else {
			internalFormat = GL_RGB16F;
		}
	} else if (format == 32) {
		if (alpha) {
			internalFormat = GL_RGBA32F;
		} else {
			internalFormat = GL_RGB32F;
		}
	}
	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

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

FrameBuffer& FrameBuffer::withTextureDepth(const uint32_t format, const bool onlyForShadowMap) {
	uint32_t internalFormat{0};

	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

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
	const int layerCount,
	const uint32_t format,
	const bool onlyForShadowMap) {
	uint32_t internalFormat{0};

	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

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

FrameBuffer& FrameBuffer::withTextureCubemapDepth(const uint32_t format, const bool onlyForShadowMap) {
	uint32_t internalFormat{0};

	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (unsigned int i = 0; i < 6; ++i)
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

FrameBuffer& FrameBuffer::withTextureCubemapDepthArray(const int layerCount, const uint32_t format,
                                                       const bool onlyForShadowMap) {
	uint32_t internalFormat{0};

	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	uint32_t textureID;
	glGenTextures(1, &textureID);
	mTextureIDs.push_back(textureID);

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

FrameBuffer& FrameBuffer::withRenderBufferDepth(const uint32_t format) {
	uint32_t internalFormat{0};
	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthMultisampled(const int multisampledCount, const uint32_t format) {
	uint32_t internalFormat{0};
	if (format == 16) {
		internalFormat = GL_DEPTH_COMPONENT16;
	} else if (format == 24) {
		internalFormat = GL_DEPTH_COMPONENT24;
	}

	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, internalFormat, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencil() {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::withRenderBufferDepthStencilMultisampled(const int multisampledCount) {
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampledCount, GL_DEPTH24_STENCIL8, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::configureAttachments() {
	if (mTextureIDs.empty()) {
		throw std::runtime_error("ERROR::FRAMEBUFFER::NO_TEXTURES_ATTACHED!\n");
	}

	glDrawBuffers(static_cast<int>(mAttachments.size()), mAttachments.data());
	return *this;
}

void FrameBuffer::checkStatus() {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR::FRAMEBUFFER::NOT_COMPLETE!\n");
	}
}

void FrameBuffer::setAttachment(const uint32_t textureID, const uint32_t target) {
	const uint32_t attachment = GL_COLOR_ATTACHMENT0 + mTextureIDs.size() - 1;
	mAttachments.push_back(attachment);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, textureID, 0);
}

void FrameBuffer::setDepthTextureParameters(const uint32_t target, const int dim) {
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

CubemapBuffer::CubemapBuffer(const int size) : mSize(size) {
	glGenFramebuffers(1, &mFBO);
	glGenRenderbuffers(1, &mRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRBO);

	glGenTextures(1, &mCubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemapID);

	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			static_cast<int32_t>(size),
			static_cast<int32_t>(size),
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
}

CubemapBuffer::~CubemapBuffer() {
	glDeleteFramebuffers(1, &mFBO);
	glDeleteRenderbuffers(1, &mRBO);
	glDeleteTextures(1, &mCubemapID);
}

uint32_t CubemapBuffer::texture() const {
	return mCubemapID;
}

void CubemapBuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, mSize, mSize);
}

void CubemapBuffer::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapBuffer::bindFace(const uint32_t face) const {
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
		mCubemapID,
		0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}
