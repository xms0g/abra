#pragma once
#include <string>
#include "glad/glad.h"
#include "command.hpp"
#include "enumUtils.hpp"
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

	explicit GraphicsEncoder(const FrameGraph& graph);

	void bindFrameBuffer(const std::string& name) const;

	void bindTexture(const std::string& name, uint32_t slot, uint32_t idx = 0) const;

	void bindPipeline(GraphicsPipeline& pipeline);

	void bindMaterial(const MaterialView& material);

	void bindTransform(const TransformView& transform) const;

	void blitFramebuffer(const std::string& src, const std::string& dst, BlitMask mask) const;

	void clearFrameBuffer(ClearMask mask) const;

	void draw(const MeshView& mesh) const;

	void drawIndexed(const MeshView& mesh) const;

	void reset();

private:
	struct EncoderState {
		MaterialCache materialCache{};
		const FrameGraph* graph{nullptr};
		GraphicsPipeline* pipeline{nullptr};
	};

	EncoderState mState{};
};
