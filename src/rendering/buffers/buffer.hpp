#pragma once
#include <cstdint>
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
		glGenBuffers(1, &mID);
	}


	Buffer(const Buffer& other) = delete;

	Buffer& operator=(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept
		: mID(std::exchange(other.mID, 0)),
		  mTarget(std::exchange(other.mTarget, 0)),
		  mUsage(other.mUsage) {
	}

	Buffer& operator=(Buffer&& other) noexcept {
		if (this != &other) {
			if (mID != 0)
				glDeleteBuffers(1, &mID);

			mID = std::exchange(other.mID, 0);
			mTarget = std::exchange(other.mTarget, 0);
			mUsage = other.mUsage;
		}

		return *this;
	}

	virtual ~Buffer() {
		if (mID != 0)
			glDeleteBuffers(1, &mID);
	}

	void bind() const {
		glBindBuffer(mTarget, mID);
	}

	void unbind() const {
		glBindBuffer(mTarget, 0);
	}

protected:
	uint32_t mID{0};
	uint32_t mTarget{0};
	BufferUsage mUsage;
};
