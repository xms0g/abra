#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"

struct RenderContext;
class Shader;
class FrameBuffer;

class OmnidirectionalShadowPass {
public:
	explicit OmnidirectionalShadowPass(const RenderContext& ctx);

	~OmnidirectionalShadowPass();

	[[nodiscard]]
	uint32_t depthTexture() const;

	[[nodiscard]]
	FrameBuffer& depthMap() const;

	void render(const RenderContext& ctx, const glm::vec4& position, int32_t layer) const;

private:
	std::unique_ptr<FrameBuffer> mDepthMap;
	std::unique_ptr<Shader> mDepthShader;

	static constexpr std::pair<glm::vec3, glm::vec3> mDirUpPairs[] {
		{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
	};
};
