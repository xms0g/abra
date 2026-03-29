#pragma once
#include <cstdint>
#include <vector>

class IBuffer {
public:
	virtual ~IBuffer() = default;

	[[nodiscard]] virtual uint32_t texture() const = 0;

	virtual void bind() const = 0;

	virtual void unbind() const = 0;

	void checkStatus();

protected:
	uint32_t mFBO{0};
	uint32_t mRBO{0};
};

class FrameBuffer final : public IBuffer {
public:
	FrameBuffer(int width, int height);

	~FrameBuffer() override;

	[[nodiscard]] int width() const;

	[[nodiscard]] int height() const;

	[[nodiscard]] uint32_t texture() const override;

	[[nodiscard]] const std::vector<uint32_t>& textures() const;

	void bind() const override;

	void bindForRead() const;

	void bindForDraw() const;

	void unbind() const override;

	void attachLayer(uint32_t attachment, int layer) const;

	FrameBuffer& withTexture();

	FrameBuffer& withTextureMultisampled(int multisampledCount);

	FrameBuffer& withTextureFP(uint32_t format, bool alpha);

	FrameBuffer& withTextureFPMultisampled(int multisampledCount, uint32_t format, bool alpha);

	FrameBuffer& withTextureDepth(uint32_t format, bool onlyForShadowMap);

	FrameBuffer& withTextureDepthArray(int layerCount, uint32_t format, bool onlyForShadowMap);

	FrameBuffer& withTextureCubemapDepth(uint32_t format, bool onlyForShadowMap);

	FrameBuffer& withTextureCubemapDepthArray(int layerCount, uint32_t format, bool onlyForShadowMap);

	FrameBuffer& withRenderBufferDepth(uint32_t format);

	FrameBuffer& withRenderBufferDepthMultisampled(int multisampledCount, uint32_t format);

	FrameBuffer& withRenderBufferDepthStencil();

	FrameBuffer& withRenderBufferDepthStencilMultisampled(int multisampledCount);

	FrameBuffer& configureAttachments();

private:
	void setAttachment(uint32_t textureID, uint32_t target);

	void setDepthTextureParameters(uint32_t target, int dim);

	int getInternalFormat(uint32_t format, bool alpha, bool depth);

	int mWidth{0};
	int mHeight{0};
	std::vector<uint32_t> mTextureIDs;
	std::vector<uint32_t> mAttachments;
};

class CubemapBuffer final : public IBuffer {
public:
	CubemapBuffer(int size);

	~CubemapBuffer() override;

	[[nodiscard]] uint32_t texture() const override;

	void bind() const override;

	void unbind() const override;

	void bindFace(uint32_t face) const;

private:
	uint32_t mCubemapID{0};
	int mSize{0};
};
