#pragma once
#include <string>
#include "glad/glad.h"
#include "command.hpp"
#include "glUtils.hpp"
#include "graphicsPipeline.h"
#include "buffers/frameBuffer.h"
#include "material/material.hpp"

class DescriptorSet;

enum class BlitMask : uint32_t {
	Color = GL_COLOR_BUFFER_BIT,
	Depth = GL_DEPTH_BUFFER_BIT,
	Stencil = GL_STENCIL_BUFFER_BIT
};

enum class ClearMask : uint32_t {
	Color = GL_COLOR_BUFFER_BIT,
	Depth = GL_DEPTH_BUFFER_BIT,
	Stencil = GL_STENCIL_BUFFER_BIT,
	None = 0
};

constexpr ClearMask operator|(const ClearMask lhs, const ClearMask rhs) {
	return static_cast<ClearMask>(toUnderlying(lhs) | toUnderlying(rhs));
}

constexpr ClearMask& operator|=(ClearMask& lhs, const ClearMask rhs) {
	lhs = static_cast<ClearMask>(toUnderlying(lhs) | toUnderlying(rhs));
	return lhs;
}

class GraphicsPipeline;
class FrameGraph;

class GraphicsEncoder {
public:
	GraphicsEncoder() = default;

	void beginRendering(const RenderingInfo& info);

	void bindFrameBuffer() const;

	void unbindFrameBuffer() const;

	void bindFrameBuffer(const FrameBuffer& frameBuffer);

	void bindVertexArray(uint32_t vao);

	void bindTexture(const TextureView& handle, uint32_t slot);

	void bindPipeline(GraphicsPipeline& pipeline);

	void pushConstants(const MaterialView& material);

	void bindTransform(const TransformView& transform);

	void bindDescriptorSet(const DescriptorSet& descriptorSet);

	void blitFramebuffer(const FrameBuffer& src, const FrameBuffer& dst, BlitMask mask) const;

	void clearFrameBuffer(ClearMask mask) const;

	void draw(size_t vertexCount) const;

	void drawIndexed(size_t indexCount) const;

	void drawInstanced(size_t indexCount, uint32_t count) const;

	void setViewport(Viewport viewport);

	void setCullFace(CullMode mode);

	void setCullMode(CullMode mode);

	template<typename T>
	void setUniform(const std::string& name, const T& value);

	template<typename T>
	void setUniform(const std::string& name, const T* value, uint32_t count) const;

	void reset();

private:
	struct HandleCache {
		uint32_t program{0};
		uint32_t vao{0};
		uint32_t framebuffer{0};
	};

	struct DepthStencilCache {
		bool depthTestEnable{};
		CompareOp depthCompareOp{};
		bool depthWriteEnable{};
		bool stencilTestEnable{};
	};

	struct CullCache {
		CullMode cullMode{};
		FrontFace frontFace{};
	};

	struct PolygonCache {
		PolygonMode polygonMode{};
		PolygonFace polygonFace{};
	};

	struct BlendCache {
		bool blendEnable{};
	};

	struct AntiAliasingCache {
		uint32_t samples{};
	};

	struct TessellationCache {
		int32_t patchVertices{};
	};

	struct GLStateCache {
		HandleCache handles{};
		DepthStencilCache depthStencil{};
		CullCache cull{};
		BlendCache blend{};
		PolygonCache polygon{};
		AntiAliasingCache aa{};
		TessellationCache tessellation{};
		Viewport viewport{};

		void reset() {
			handles = {};
			depthStencil = {};
			cull = {};
			blend = {};
			polygon = {};
			aa = {};
			tessellation = {};
			viewport = {};
		}
	};

	struct EncoderState {
		TextureBindingCache bindingCache{};
		GLStateCache glStateCache{};
		GraphicsPipeline* pipeline{nullptr};
	};

	EncoderState mState{};
};

template<typename T>
void GraphicsEncoder::setUniform(const std::string& name, const T& value) {
	assert(mState.pipeline);
	mState.pipeline->setValue(name, value);
}

template<typename T>
void GraphicsEncoder::setUniform(const std::string& name, const T* value, uint32_t count) const {
	assert(mState.pipeline);
	mState.pipeline->setValue(name, value, count);
}
