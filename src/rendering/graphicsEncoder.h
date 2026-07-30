#pragma once
#include <string>
#include "glad/glad.h"
#include "command.hpp"
#include "enumUtils.hpp"
#include "graphicsPipeline.h"
#include "buffers/frameBuffer.h"
#include "material/material.hpp"

enum class BlitMask : uint32_t {
	Color = GL_COLOR_BUFFER_BIT,
	Depth = GL_DEPTH_BUFFER_BIT,
	Stencil = GL_STENCIL_BUFFER_BIT
};

enum class ClearMask : uint32_t {
	Color = GL_COLOR_BUFFER_BIT,
	Depth = GL_DEPTH_BUFFER_BIT,
	Stencil = GL_STENCIL_BUFFER_BIT,
};

constexpr ClearMask operator|(const ClearMask lhs, const ClearMask rhs) {
	return static_cast<ClearMask>(toUnderlying(lhs) | toUnderlying(rhs));
}

class GraphicsPipeline;
class FrameGraph;

class GraphicsEncoder {
public:
	GraphicsEncoder() = default;

	void bindFrameBuffer() const;

	void unbindFrameBuffer() const;

	void bindFrameBuffer(const FrameBuffer& fb) const;

	void bindTexture(TextureHandle handle, uint32_t slot) const;

	void bindPipeline(GraphicsPipeline& pipeline);

	void bindMaterial(const MaterialView& material);

	void bindTransform(const TransformView& transform);

	void blitFramebuffer(const FrameBuffer& src, const FrameBuffer& dst, BlitMask mask) const;

	void clearFrameBuffer(ClearMask mask) const;

	void draw(const MeshView& mesh) const;

	void drawIndexed(const MeshView& mesh) const;

	void drawInstanced(const MeshView& mesh, uint32_t count) const;

	void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) const;

	void setCullMode(CullMode mode);

	template<typename T>
	void setUniform(const std::string& name, const T& value);

	template<typename T>
	void setUniform(const std::string& name, const T* value, uint32_t count) const;

	void reset();

private:
	struct EncoderState {
		MaterialCache materialCache{};
		GraphicsPipeline* pipeline{nullptr};
	};

	EncoderState mState{};
};

template<typename T>
void GraphicsEncoder::setUniform(const std::string& name, const T& value) {
	assert(mState.pipeline);
	mState.pipeline->state().shader.setValue(name, value);
}

template<typename T>
void GraphicsEncoder::setUniform(const std::string& name, const T* value, uint32_t count) const {
	assert(mState.pipeline);
	mState.pipeline->state().shader.setValue(name, value, count);
}
