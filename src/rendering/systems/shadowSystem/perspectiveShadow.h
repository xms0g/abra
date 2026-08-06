#pragma once
#include <span>
#include "glm/glm.hpp"

class GraphicsEncoder;
class GraphicsPipeline;
struct RenderGroup;
struct RenderContext;
class Shader;
class FrameBuffer;

class PerspectiveShadow {
public:
	explicit PerspectiveShadow(const RenderContext& ctx);

	~PerspectiveShadow();

	[[nodiscard]]
	glm::mat4 lightSpaceMatrix(int layer) const;

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const FrameBuffer& frameBuffer,
	            const glm::vec3& direction,
	            const glm::vec3& position,
	            float fovy,
	            int32_t layer);

private:
	int32_t mWidth{0}, mHeight{0};
	float mAspect{0.0f}, mFar{0.0f}, mNear{0.0f};
	glm::mat4 mLightSpaceMatrix[4]{};
	std::span<RenderGroup> mObjects;
};
