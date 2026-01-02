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

	struct {
		const FrameBuffer* buffer;
		int kernelSize;
		int noiseTextureSize;
		float radius;
		float bias;
		float intensity;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
	} ssao;

	struct {
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
		uint32_t maxDirLights, maxPointLights, maxSpotLights;
		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
	} light;

	struct {
		const Camera* self;
		const math::Frustum* frustum;
		glm::mat4 skyView;
		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
	} camera;

	struct {
		uint32_t width, height;
	} screen;

	struct {
		const std::array<uint32_t, 3>* textures;
		int textureSlot;
		int width, height;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		struct  {
			int maxLights;
			float height, nearPlane, farPlane, left, right, bottom, top;
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