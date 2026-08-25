#include "vertexBuffer.hpp"

VertexBuffer::VertexBuffer(const int32_t size, const BufferUsage usage)
	: GPUBuffer(BufferType::Vertex, size, usage) {
	if (size > 0) {
		bind();
		glBufferData(std::to_underlying(mTarget), size, nullptr, std::to_underlying(mUsage));
		unbind();
	}
}

void VertexBuffer::copyToMemory(const void* data, const uint32_t offset, const uint32_t size) const {
	bind();

	switch (mUsage) {
		case BufferUsage::Static:
			glBufferData(std::to_underlying(mTarget), size, data, std::to_underlying(mUsage));
			break;
		case BufferUsage::Dynamic: {
			glBufferSubData(std::to_underlying(mTarget), offset, size, data);
			break;
		}
		default: break;
	}
}
