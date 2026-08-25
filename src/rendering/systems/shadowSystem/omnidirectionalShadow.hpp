#pragma once
#include <span>
#include "glm/glm.hpp"
#include "data.hpp"
#include "../../context/renderQueue.hpp"

class UniformBuffer;
class GraphicsEncoder;
class GraphicsPipeline;
struct RenderGroup;
struct RenderContext;

class OmnidirectionalShadow {
public:
	explicit OmnidirectionalShadow(const RenderContext& ctx);

	~OmnidirectionalShadow();

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const UniformBuffer& ubo,
	            const glm::vec3& position,
	            int32_t layer);

private:
	OmnidirectionalShadowData mData{};
	std::span<RenderGroup> mObjects;
	glm::mat4 mShadowProj{};

	struct DirUpPair {
		glm::vec3 dir;
		glm::vec3 up;
	};

	static constexpr uint32_t faces = 6;
	static constexpr DirUpPair mDirUps[faces] = {
		{.dir = glm::vec3(1.0f, 0.0f, 0.0f), .up = glm::vec3(0.0f, -1.0f, 0.0f)},
		{.dir = glm::vec3(-1.0f, 0.0f, 0.0f), .up = glm::vec3(0.0f, -1.0f, 0.0f)},
		{.dir = glm::vec3(0.0f, 1.0f, 0.0f), .up = glm::vec3(0.0f, 0.0f, 1.0f)},
		{.dir = glm::vec3(0.0f, -1.0f, 0.0f), .up = glm::vec3(0.0f, 0.0f, -1.0f)},
		{.dir = glm::vec3(0.0f, 0.0f, 1.0f), .up = glm::vec3(0.0f, -1.0f, 0.0f)},
		{.dir = glm::vec3(0.0f, 0.0f, -1.0f), .up = glm::vec3(0.0f, -1.0f, 0.0f)}
	};
};
