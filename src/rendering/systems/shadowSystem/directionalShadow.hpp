#pragma once
#include <span>
#include "glm/glm.hpp"
#include "data.hpp"

class UniformBuffer;
class GraphicsPipeline;
class GraphicsEncoder;
struct RenderGroup;
struct RenderContext;

class DirectionalShadow {
public:
	explicit DirectionalShadow(const RenderContext& ctx);

	~DirectionalShadow();

	void render(const RenderContext& ctx,
	            GraphicsEncoder& encoder,
	            GraphicsPipeline& pipeline,
	            const UniformBuffer& ubo,
	            const glm::vec3& direction);

private:
	float mHeight;

	glm::mat4 mLightProjection{};
	DirectionalShadowData mData{};
	std::span<RenderGroup> mObjects;
};
