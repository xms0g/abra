#pragma once
#include <array>
#include <vector>
#include "glm/glm.hpp"

namespace math {
struct Frustum;
}

class FrameBuffer;
class UniformBuffer;
class Camera;
struct RenderQueue;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	const RenderQueue* renderQueue;
	mutable const FrameBuffer* sceneBuffer;
	const FrameBuffer* intermediateBuffer;

	struct {
		const UniformBuffer* ubo;
		uint32_t maxDirLights, maxPointLights, maxSpotLights;
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
	} light;

	struct {
		const UniformBuffer* ubo;
		const Camera* self;
		const math::Frustum* frustum;
		glm::mat4 skyView;
	} camera;

	struct {
		uint32_t width, height;
	} screen;

	struct {
		uint32_t textureSlot;
		uint32_t width, height;

		struct  {
			float nearPlane, farPlane, left, right, bottom, top;
		} directional;

		struct {
			uint32_t maxLights;
			float nearPlane, farPlane, fovy;
		} omnidirectional;

		struct {
			uint32_t maxLights;
			float nearPlane, farPlane;
		} perspective;

		const std::array<uint32_t, 3>* textures;
		const UniformBuffer* ubo;
	} shadowMap;

	RenderContext() = default;
};