#pragma once
#include <cstdint>
#include <vector>
#include "../texture/texture.h"

enum class Attachment : uint32_t {
	Color0 = GL_COLOR_ATTACHMENT0,
	Depth = GL_DEPTH_ATTACHMENT,
};

enum class BaseFormat : int32_t {
	Red = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGBA = GL_RGBA,
};

enum class InternalFormat : int32_t {
	Red = GL_R8,
	RG = GL_RG8,
	RGB = GL_RGB8,
	RGBA = GL_RGBA8,
	RedFloat = GL_R16F,
	RGFloat = GL_RG16F,
	RGBFloat = GL_RGB16F,
	RGBAFloat = GL_RGBA16F,
	Depth24 = GL_DEPTH_COMPONENT24,
	Depth32F = GL_DEPTH_COMPONENT32F,
};

class FrameBuffer {
public:
	FrameBuffer(int32_t width, int32_t height);

	FrameBuffer(const FrameBuffer&) = delete;

	FrameBuffer& operator=(const FrameBuffer&) = delete;

	FrameBuffer(FrameBuffer&& other) noexcept;

	FrameBuffer& operator=(FrameBuffer&& other) noexcept;

	~FrameBuffer();

	[[nodiscard]]
	int32_t width() const;

	[[nodiscard]]
	int32_t height() const;

	[[nodiscard]]
	TextureView texture(uint32_t index = 0) const;

	void bind() const;

	void unbind() const;

	void resizeRenderBuffer(int32_t width, int32_t height) const;

	void attachTexture(uint32_t index, Attachment attachment, int32_t mip, int32_t layer) const;

	void checkStatus();

	void bindForRead() const;

	void bindForDraw() const;

	FrameBuffer& withTexture(BaseFormat format);

	FrameBuffer& withTextureMultisampled(int32_t multisampledCount, BaseFormat format);

	FrameBuffer& withTextureFP(BaseFormat format);

	FrameBuffer& withTextureFPMultisampled(int32_t multisampledCount, BaseFormat format);

	FrameBuffer& withTextureDepth(InternalFormat format, bool onlyForShadowMap);

	FrameBuffer& withTextureDepthArray(int32_t layerCount, InternalFormat format, bool onlyForShadowMap);

	FrameBuffer& withTextureCubeMap();

	FrameBuffer& withTextureCubemapDepth(InternalFormat format, bool onlyForShadowMap);

	FrameBuffer& withTextureCubemapDepthArray(int32_t layerCount, InternalFormat format, bool onlyForShadowMap);

	FrameBuffer& withRenderBufferDepth(InternalFormat format);

	FrameBuffer& withRenderBufferDepthMultisampled(int32_t multisampledCount, InternalFormat format);

	FrameBuffer& withRenderBufferDepthStencil(InternalFormat format);

	FrameBuffer& withRenderBufferDepthStencilMultisampled(int32_t multisampledCount, InternalFormat format);

	FrameBuffer& configureAttachments();

private:
	void setAttachment(uint32_t textureID, uint32_t target);

	static void setDepthTextureParameters(uint32_t target, int32_t dim);

	static InternalFormat getInternalFormat(BaseFormat format, bool isFloat = false);

	uint32_t mFBO{0}, mRBO{0};
	int32_t mWidth{0}, mHeight{0};

	struct TextureDescription {
		uint32_t id{0};
		TextureTarget target{};
	};

	std::vector<TextureDescription> mTextures;
	std::vector<uint32_t> mAttachments;
};
