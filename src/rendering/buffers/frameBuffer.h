#pragma once
#include <cstdint>
#include <vector>

class BaseFrameBuffer {
public:
	BaseFrameBuffer(int32_t width, int32_t height);

	virtual ~BaseFrameBuffer();

	[[nodiscard]]
	int32_t width() const;

	[[nodiscard]]
	int32_t height() const;

	[[nodiscard]]
	uint32_t texture(const uint32_t index = 0) const {
		return textureImpl(index);
	}

	void bind() const;

	void unbind();

	void bindTexture(const uint32_t slot, const uint32_t index = 0) const {
		bindTextureImpl(slot, index);
	}

	void resizeRenderBuffer(int32_t width, int32_t height) const;

	void checkStatus();

	virtual void generateMipmaps() = 0;

protected:
	[[nodiscard]]
	virtual uint32_t textureImpl(uint32_t index) const = 0;

	virtual void bindTextureImpl(uint32_t slot, uint32_t index) const = 0;

	uint32_t mFBO{0};
	uint32_t mRBO{0};
	int32_t mWidth{0};
	int32_t mHeight{0};

};

class FrameBuffer final : public BaseFrameBuffer {
public:
	FrameBuffer(int32_t width, int32_t height);

	~FrameBuffer() override;

	void bindForRead() const;

	void bindForDraw() const;

	FrameBuffer& withTexture(uint32_t format);

	FrameBuffer& withTextureMultisampled(int32_t multisampledCount, uint32_t format);

	FrameBuffer& withTextureFP(uint32_t format);

	FrameBuffer& withTextureFPMultisampled(int32_t multisampledCount, uint32_t format);

	FrameBuffer& withTextureDepth(int32_t internalFormat, bool onlyForShadowMap);

	FrameBuffer& withTextureDepthArray(int32_t layerCount, int32_t internalFormat, bool onlyForShadowMap);

	FrameBuffer& withTextureCubemapDepth(int32_t internalFormat, bool onlyForShadowMap);

	FrameBuffer& withTextureCubemapDepthArray(int32_t layerCount, int32_t internalFormat, bool onlyForShadowMap);

	FrameBuffer& withRenderBufferDepth(uint32_t internalFormat);

	FrameBuffer& withRenderBufferDepthMultisampled(int32_t multisampledCount, uint32_t internalFormat);

	FrameBuffer& withRenderBufferDepthStencil(int32_t internalFormat);

	FrameBuffer& withRenderBufferDepthStencilMultisampled(int32_t multisampledCount, int32_t internalFormat);

	FrameBuffer& configureAttachments();

	void generateMipmaps() override;

protected:
	[[nodiscard]]
	uint32_t textureImpl(uint32_t index) const override;

	void bindTextureImpl(uint32_t slot, uint32_t textureIndex) const override;

private:
	void setAttachment(uint32_t textureID, uint32_t target);

	static void setDepthTextureParameters(uint32_t target, int32_t dim);

	static int32_t getInternalFormat(uint32_t format, bool isFloat = false);

	struct TextureDescription {
		uint32_t id{0};
		uint32_t target{0};
	};
	
	std::vector<TextureDescription> mTextures;
	std::vector<uint32_t> mAttachments;
};

class CubemapBuffer final : public BaseFrameBuffer {
public:
	CubemapBuffer(int32_t width, int32_t height, bool mipmap = false, bool prefilter = false);

	~CubemapBuffer() override;

	void bindFace(uint32_t face, int32_t mip = 0) const;

	void generateMipmaps() override;

protected:
	[[nodiscard]]
	uint32_t textureImpl(uint32_t index) const override;

	void bindTextureImpl(uint32_t slot, uint32_t textureIndex) const override;

private:
	uint32_t mCubemapID{0};
};
