#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"

struct RenderContext;
class Shader;
class FrameBuffer;

class DirectionalShadowPass {
public:
	explicit DirectionalShadowPass(const RenderContext& ctx);

	~DirectionalShadowPass();

	[[nodiscard]] uint32_t getDepthTexture() const;

	[[nodiscard]] glm::mat4 getLightSpaceMatrix() const;

	void render(const RenderContext& ctx, const glm::vec4& direction);

private:
	glm::mat4 mLightSpaceMatrix{};
	std::unique_ptr<FrameBuffer> mDepthMap;
	std::unique_ptr<Shader> mDepthShader;
};
