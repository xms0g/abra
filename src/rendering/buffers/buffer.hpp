#pragma once
#include <utility>
#include "glad/glad.h"
#include "../glUtils.hpp"

enum class BufferUsage: uint32_t {
	Static = GL_STATIC_DRAW,
	Dynamic = GL_DYNAMIC_DRAW,
	Stream = GL_STREAM_DRAW
};

enum class BufferType: uint32_t {
	Vertex = GL_ARRAY_BUFFER,
	Index = GL_ELEMENT_ARRAY_BUFFER,
	Uniform = GL_UNIFORM_BUFFER
};

class Buffer {
public:
	Buffer() = default;

	Buffer(const BufferType target, const int32_t size, const BufferUsage usage)
		: mTarget(target),
		  mSize(size),
		  mUsage(usage) {
		glGenBuffers(1, &mHandle);
	}


	Buffer(const Buffer& other) = delete;

	Buffer& operator=(const Buffer& other) = delete;

	Buffer(Buffer&& other) noexcept
		: mHandle(std::exchange(other.mHandle, 0)),
		  mTarget(std::exchange(other.mTarget, {})),
		  mSize(std::exchange(other.mSize, 0)),
		  mUsage(other.mUsage) {
	}

	Buffer& operator=(Buffer&& other) noexcept {
		if (this != &other) {
			if (mHandle)
				glDeleteBuffers(1, &mHandle);

			mHandle = std::exchange(other.mHandle, 0);
			mTarget = std::exchange(other.mTarget, {});
			mSize = std::exchange(other.mSize, 0);
			mUsage = other.mUsage;
		}

		return *this;
	}

	virtual ~Buffer() {
		if (mHandle)
			glDeleteBuffers(1, &mHandle);
	}

	[[nodiscard]]
	uint32_t handle() const {
		return mHandle;
	}

	[[nodiscard]]
	BufferType target() const {
		return mTarget;
	}

	[[nodiscard]]
	int32_t size() const {
		return mSize;
	}

	void bind() const {
		glBindBuffer(toUnderlying(mTarget), mHandle);
	}

	void unbind() const {
		glBindBuffer(toUnderlying(mTarget), 0);
	}

protected:
	uint32_t mHandle{0};
	BufferType mTarget{};
	int32_t mSize{0};
	BufferUsage mUsage{};
};
