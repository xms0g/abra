#pragma once
#include <span>
#include "glm/glm.hpp"
#include "../../context/renderQueue.hpp"

class GraphicsPipeline;
class GraphicsEncoder;
class FrameGraph;
struct RenderGroup;
struct RenderContext;
class Shader;
class FrameBuffer;

class DirectionalShadow {
public:
	explicit DirectionalShadow(const RenderContext& ctx);

	~DirectionalShadow();

	[[nodiscard]]
	glm::mat4 lightSpaceMatrix() const;

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const glm::vec3& direction);

private:
	float mHeight, mLeft, mRight, mBottom, mTop, mNear, mFar;
	glm::mat4 mLightSpaceMatrix{};
	std::span<RenderGroup> mObjects;
};
