#pragma once
#include <span>
#include "glm/glm.hpp"
#include "../../context/renderQueue.hpp"

class GraphicsEncoder;
class GraphicsPipeline;
struct RenderGroup;
struct RenderContext;
class FrameBuffer;

class OmnidirectionalShadow {
public:
	explicit OmnidirectionalShadow(const RenderContext& ctx);

	~OmnidirectionalShadow();

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const glm::vec3& position,
	            int32_t layer);

private:
	int32_t mWidth{0}, mHeight{0};
	float mAspect{0.0f}, mFar{0.0f}, mNear{0.0f}, mFovy{0.0f};

	std::vector<glm::mat4> mShadowTransforms;
	std::span<RenderGroup> mObjects;
	glm::mat4 mShadowProj{};

	struct DirUpPair {
		glm::vec3 dir;
		glm::vec3 up;
	};

	static constexpr uint32_t faces = 6;
	static constexpr DirUpPair mDirUps[faces] = {
		{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
	};
};
