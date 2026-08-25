#pragma once
#include <span>
#include "glm/glm.hpp"
#include "data.hpp"

class UniformBuffer;
class GraphicsEncoder;
class GraphicsPipeline;
struct RenderGroup;
struct RenderContext;
class FrameBuffer;

class PerspectiveShadow {
public:
	explicit PerspectiveShadow(const RenderContext& ctx);

	~PerspectiveShadow();

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const FrameBuffer& frameBuffer,
	            const UniformBuffer& ubo,
	            const glm::vec3& direction,
	            const glm::vec3& position,
	            float fovy,
	            int32_t layer);

private:
	float mAspect{0.0f};
	float mFar{0.0f};
	float mNear{0.0f};
	PerspectiveShadowData mData{};
	std::span<RenderGroup> mObjects;
};
