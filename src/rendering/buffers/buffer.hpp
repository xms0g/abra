#pragma once
#include <utility>
#include "glad/glad.h"

enum BufferUsage {
	STATIC = GL_STATIC_DRAW,
	DYNAMIC = GL_DYNAMIC_DRAW,
	STREAM = GL_STREAM_DRAW
};

class Buffer {
public:
	Buffer() = default;

	Buffer(const uint32_t target, const BufferUsage usage)
		: mTarget(target),
		  mUsage(usage) {
		glGenBuffers(1, &mHandle);
	}


	Buffer(const Buffer& other) = delete;

	Buffer& operator=(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept
		: mHandle(std::exchange(other.mHandle, 0)),
		  mTarget(std::exchange(other.mTarget, 0)),
		  mUsage(other.mUsage) {
	}

	Buffer& operator=(Buffer&& other) noexcept {
		if (this != &other) {
			if (mHandle != 0)
				glDeleteBuffers(1, &mHandle);

			mHandle = std::exchange(other.mHandle, 0);
			mTarget = std::exchange(other.mTarget, 0);
			mUsage = other.mUsage;
		}

		return *this;
	}

	virtual ~Buffer() {
		if (mHandle != 0)
			glDeleteBuffers(1, &mHandle);
	}

	[[nodiscard]]
	uint32_t id() const {
		return mHandle;
	}

	[[nodiscard]]
	uint32_t target() const {
		return mTarget;
	}

	void bind() const {
		glBindBuffer(mTarget, mHandle);
	}

	void unbind() const {
		glBindBuffer(mTarget, 0);
	}

protected:
	uint32_t mHandle{0};
	uint32_t mTarget{0};
	BufferUsage mUsage{};
};
