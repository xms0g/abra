#pragma once
#include <cstdint>
#include <vector>

class IFrameBuffer {
public:
	virtual ~IFrameBuffer() = default;

	[[nodiscard]]
	virtual uint32_t texture(uint32_t index) const = 0;

	virtual void bind() const = 0;

	virtual void unbind() const = 0;

	virtual void bindTexture(uint32_t slot, uint32_t index) const = 0;

	void checkStatus();

protected:
	uint32_t mFBO{0};
	uint32_t mRBO{0};
};

class FrameBuffer final : public IFrameBuffer {
public:
	FrameBuffer(int32_t width, int32_t height);

	~FrameBuffer() override;

	[[nodiscard]]
	int32_t width() const;

	[[nodiscard]]
	int32_t height() const;

	[[nodiscard]]
	uint32_t texture(uint32_t index = 0) const override;

	[[nodiscard]]
	const std::vector<std::pair<uint32_t, uint32_t>>& textures() const;

	void bind() const override;

	void bindForRead() const;

	void bindForDraw() const;

	void unbind() const override;

	void bindTexture(uint32_t slot, uint32_t textureIndex = 0) const override;

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

private:
	void setAttachment(uint32_t textureID, uint32_t target);

	void setDepthTextureParameters(uint32_t target, int32_t dim);

	int32_t getInternalFormat(uint32_t format, bool isFloat = false);

	int32_t mWidth{0};
	int32_t mHeight{0};
	std::vector<std::pair<uint32_t, uint32_t>> mTextures;
	std::vector<uint32_t> mAttachments;
};

class CubemapBuffer final : public IFrameBuffer {
public:
	CubemapBuffer(int32_t size, bool mipmap = false, bool prefilter = false);

	~CubemapBuffer() override;

	[[nodiscard]]
	uint32_t texture(uint32_t index = 0) const override;

	[[nodiscard]]
	uint32_t rbo() const;

	void bind() const override;

	void unbind() const override;

	void bindFace(uint32_t face, int32_t mip = 0) const;

	void bindTexture(uint32_t slot, uint32_t textureIndex = 0) const override;

private:
	uint32_t mCubemapID{0};
	int32_t mSize{0};
};
