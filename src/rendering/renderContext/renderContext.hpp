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
	RenderQueue* renderQueue;
	mutable const FrameBuffer* sceneBuffer;
	const FrameBuffer* intermediateBuffer;
	const FrameBuffer* gBuffer;
	const FrameBuffer* ssao;

	struct {
		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
		uint32_t maxDirLights, maxPointLights, maxSpotLights;
	} light;

	struct {
		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		const Camera* self;
		const math::Frustum* frustum;
		glm::mat4 skyView;
	} camera;

	struct {
		uint32_t width, height;
	} screen;

	struct {
		const std::array<uint32_t, 3>* textures;
		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		int textureSlot;
		int width, height;

		struct  {
			int maxLights;
			float nearPlane, farPlane, left, right, bottom, top;
		} directional;

		struct {
			int maxLights;
			float nearPlane, farPlane, fovy;
		} omnidirectional;

		struct {
			int maxLights;
			float nearPlane, farPlane;
		} perspective;
	} shadow;

	RenderContext() = default;
	RenderContext(const RenderContext&) = delete;
	RenderContext& operator=(const RenderContext&) = delete;
};