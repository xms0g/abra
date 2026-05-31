#pragma once
#include <memory>
#include "glm/glm.hpp"

struct RenderContext;
class Shader;
class FrameBuffer;

class OmnidirectionalShadow {
public:
	explicit OmnidirectionalShadow(const RenderContext& ctx);

	~OmnidirectionalShadow();

	[[nodiscard]]
	uint32_t depthTexture() const;

	[[nodiscard]]
	FrameBuffer& depthMap() const;

	void render(const RenderContext& ctx, const glm::vec3& position, int32_t layer) const;

private:
	std::unique_ptr<FrameBuffer> mDepthMap;
	const Shader* mDepthShader;

	static constexpr std::pair<glm::vec3, glm::vec3> mDirUpPairs[] {
		{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
	};
};
