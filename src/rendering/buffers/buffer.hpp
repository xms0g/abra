#pragma once
#include "glad/glad.h"

enum BufferUsage {
	STATIC = GL_STATIC_DRAW,
	DYNAMIC = GL_DYNAMIC_DRAW,
	STREAM = GL_STREAM_DRAW
};

class Buffer {
public:
	Buffer(const uint32_t target, const BufferUsage usage)
		: mTarget(target), mUsage(usage) {
		glGenBuffers(1, &mID);
	}

	virtual ~Buffer() {
		glDeleteBuffers(1, &mID);
	}

	void bind() const {
		glBindBuffer(mTarget, mID);
	}

	void unbind() const {
		glBindBuffer(mTarget, 0);
	}

protected:
	uint32_t mID;
	uint32_t mTarget;
	BufferUsage mUsage;
};
