#pragma once
#include <cstdint>
#include <vector>

class FrameBuffer {
public:
	FrameBuffer(int width, int height);

	~FrameBuffer();

	[[nodiscard]] int width() const { return mWidth; }
	[[nodiscard]] int height() const { return mHeight; }
	[[nodiscard]] uint32_t texture() const { return mTextureIDs.front(); }
	[[nodiscard]] const std::vector<uint32_t>& textures() const { return mTextureIDs; }

	void bind() const;

	void bindForRead() const;

	void bindForDraw() const;

	void unbind() const;

	void attachLayer(uint32_t attachment, int layer) const;

	FrameBuffer& withTexture();

	FrameBuffer& withTextureMultisampled(int multisampledCount);

	FrameBuffer& withTextureFP(bool alpha, uint32_t format);

	FrameBuffer& withTextureFPMultisampled(bool alpha, uint32_t format, int multisampledCount);

	FrameBuffer& withTextureDepth();

	FrameBuffer& withTextureArrayDepth(int layerCount);

	FrameBuffer& withTextureCubemapDepth();

	FrameBuffer& withTextureCubemapArrayDepth(int layerCount);

	FrameBuffer& withRenderBufferDepth(uint32_t depthFormat);

	FrameBuffer& withRenderBufferDepthMultisampled(int multisampledCount, uint32_t depthFormat);

	FrameBuffer& withRenderBufferDepthStencil();

	FrameBuffer& withRenderBufferDepthStencilMultisampled(int multisampledCount);

	FrameBuffer& configureAttachments();

	void checkStatus();

private:
	void setAttachment(uint32_t textureID, uint32_t target);

	void setDepthTextureParameters(uint32_t target, int dim);

	int mWidth{0};
	int mHeight{0};
	uint32_t mFBO{0};
	uint32_t mRBO{0};
	std::vector<uint32_t> mTextureIDs;
	std::vector<uint32_t> mAttachments;
};
