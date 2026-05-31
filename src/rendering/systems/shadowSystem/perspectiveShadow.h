#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../../../config/config.hpp"

struct RenderContext;
class Shader;
class FrameBuffer;

class PerspectiveShadow {
public:
	explicit PerspectiveShadow(const RenderContext& ctx);

	~PerspectiveShadow();

	[[nodiscard]]
	uint32_t depthTexture() const;

	[[nodiscard]]
	FrameBuffer& depthMap() const;

	[[nodiscard]]
	glm::mat4 lightSpaceMatrix(int layer) const;

	void render(
		const RenderContext& ctx,
		const glm::vec3& direction,
		const glm::vec3& position,
		float fovy,
		int32_t layer);

private:
	glm::mat4 mLightSpaceMatrix[MAX_SPOT_LIGHTS]{};
	std::unique_ptr<FrameBuffer> mDepthMap;
	const Shader* mDepthShader;
};
