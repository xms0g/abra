#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"

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

	void render(
		const RenderContext& ctx,
		const glm::vec3& direction,
		const glm::vec3& position,
		float fovy,
		uint32_t texture,
		int32_t layer);

private:
	int32_t mWidth{0};
	int32_t mHeight{0};
	float mAspect{0.0f};
	float mFar{0.0f};
	float mNear{0.0f};
	glm::mat4 mLightSpaceMatrix[4]{};
	const Shader* mDepthShader;
	std::vector<RenderGroup>* mObjects;
};
