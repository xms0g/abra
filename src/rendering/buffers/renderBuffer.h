#pragma once
#include <cstdint>
#include "../texture.h"

class RenderBuffer {
public:
	RenderBuffer() = default;

	RenderBuffer(InternalFormat format, int32_t width, int32_t height);

	RenderBuffer(InternalFormat format, int32_t width, int32_t height, int32_t samples);

	~RenderBuffer();

	RenderBuffer(const RenderBuffer& other) = delete;

	RenderBuffer& operator=(const RenderBuffer& other) = delete;

	RenderBuffer(RenderBuffer&& other) noexcept;

	RenderBuffer& operator=(RenderBuffer&& other) noexcept;

	[[nodiscard]]
	uint32_t handle() const;

	void resize(int32_t width, int32_t height) const;

private:
	uint32_t mHandle{0};
	InternalFormat mFormat{};
};
