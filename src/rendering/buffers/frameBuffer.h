#pragma once
#include <vector>
#include "../texture.h"

class RenderBuffer;

enum class Attachment : uint32_t {
	Color0 = GL_COLOR_ATTACHMENT0,
	Depth = GL_DEPTH_ATTACHMENT,
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
	uint32_t handle() const;

	[[nodiscard]]
	int32_t width() const;

	[[nodiscard]]
	int32_t height() const;

	[[nodiscard]]
	const std::shared_ptr<GPUTexture>& texture(uint32_t index = 0) const;

	RenderBuffer& renderBuffer(uint32_t index = 0);

	FrameBuffer& attachColor(std::shared_ptr<GPUTexture>& texture);

	FrameBuffer& attachDepth(std::shared_ptr<GPUTexture>& texture);

	FrameBuffer& attachDepth(RenderBuffer& renderBuffer);

	FrameBuffer& attachDepthStencil(RenderBuffer& renderBuffer);

	FrameBuffer& configureDrawBuffers();

	void checkStatus();

private:

	uint32_t mHandle{0};
	int32_t mWidth{0};
	int32_t mHeight{0};
	uint32_t mColorAttachmentCount{0};
	std::vector<std::shared_ptr<GPUTexture>> mTextures;
	std::vector<RenderBuffer> mRenderBuffers;
};
