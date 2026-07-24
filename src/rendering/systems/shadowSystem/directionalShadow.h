#pragma once
#include "glm/glm.hpp"
#include "../../context/renderQueue.hpp"

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

	void render(const RenderContext& ctx, const FrameGraph& graph, const glm::vec3& direction);

private:
	float mHeight, mLeft, mRight, mBottom, mTop, mNear, mFar;
	glm::mat4 mLightSpaceMatrix{};
	const Shader* mDepthShader;
	RenderQueue<RenderGroup>* mObjects;
};
