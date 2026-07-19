#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"

struct RenderGroup;
struct RenderContext;
class Shader;
class FrameBuffer;

class OmnidirectionalShadow {
public:
	explicit OmnidirectionalShadow(const RenderContext& ctx);

	~OmnidirectionalShadow();

	void render(const RenderContext& ctx, const glm::vec3& position, int32_t layer);

private:
	int32_t mWidth{0};
	int32_t mHeight{0};
	float mAspect{0.0f};
	float mFar{0.0f};
	float mNear{0.0f};
	float mFovy{0.0f};

	std::vector<glm::mat4> mShadowTransforms;
	std::vector<RenderGroup>* mObjects;
	const Shader* mDepthShader;
	glm::mat4 mShadowProj;

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
