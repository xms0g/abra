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
		uint32_t kernelSize;
		uint32_t noiseTextureSize;
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
		uint32_t textureSlot;
		uint32_t width, height;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		struct  {
			uint32_t maxLights;
			float height, nearPlane, farPlane, left, right, bottom, top;
		} directional;

		struct {
			uint32_t maxLights;
			float nearPlane, farPlane, fovy;
		} omnidirectional;

		struct {
			uint32_t maxLights;
			float nearPlane, farPlane;
		} perspective;
	} shadow;

	struct {
		struct {
			mutable uint32_t binding;
			uint32_t size;
		} envMap;

		struct {
			uint32_t textureSlot;
			uint32_t size;
		} irradianceMap;

		struct {
			uint32_t textureSlot;
			uint32_t size;
		} prefilterMap;

		struct {
			uint32_t textureSlot;
			uint32_t size;
		} brdfLUT;

		const char* HDRTexture;
	} PBR;

	RenderContext() = default;
	RenderContext(const RenderContext&) = delete;
	RenderContext& operator=(const RenderContext&) = delete;
};