#pragma once
#include <span>
#include "glm/glm.hpp"

class GraphicsPipeline;
class GraphicsEncoder;
struct RenderGroup;
struct RenderContext;

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
	float mHeight;
	float mLeft;
	float mRight;
	float mBottom;
	float mTop;
	float mNear;
	float mFar;
	glm::mat4 mLightSpaceMatrix{};
	std::span<RenderGroup> mObjects;
};
