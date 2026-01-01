#pragma once
#include <cstdint>
#include <vector>

class FrameBuffer {
public:
	FrameBuffer(int width, int height);

	~FrameBuffer();

	[[nodiscard]] int width() const;

	[[nodiscard]] int height() const;

	[[nodiscard]] uint32_t texture() const;

	[[nodiscard]] const std::vector<uint32_t>& textures() const;

	void bind() const;

	void bindForRead() const;

	void bindForDraw() const;

	void unbind() const;

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
