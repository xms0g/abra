#pragma once
#include <memory>
#include "glm/glm.hpp"

struct RenderContext;
class Shader;
class FrameBuffer;

class DirectionalShadow {
public:
	explicit DirectionalShadow(const RenderContext& ctx);

	~DirectionalShadow();

	[[nodiscard]]
	uint32_t depthTexture() const;

	[[nodiscard]]
	glm::mat4 lightSpaceMatrix() const;

	void render(const RenderContext& ctx, const glm::vec3& direction);

private:
	float mHeight, mLeft, mRight, mBottom, mTop, mNear, mFar;
	glm::mat4 mLightSpaceMatrix{};
	std::unique_ptr<FrameBuffer> mDepthMap;
	const Shader* mDepthShader;
};
