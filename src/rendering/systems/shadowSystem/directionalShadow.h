#pragma once
#include "glm/glm.hpp"
#include "../../renderContext/renderQueue.hpp"

class RenderGraph;
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

	void render(const RenderContext& ctx, const RenderGraph& graph, const glm::vec3& direction);

private:
	float mHeight, mLeft, mRight, mBottom, mTop, mNear, mFar;
	glm::mat4 mLightSpaceMatrix{};
	const Shader* mDepthShader;
	RenderQueue<RenderGroup>* mObjects;
};
