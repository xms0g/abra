#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../../config/config.hpp"
#include "../material/material.hpp"

namespace math {
struct Frustum;
}

class BaseFrameBuffer;
class FrameBuffer;
class UniformBuffer;
class Camera;
class Shader;
struct RenderQueue;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderQueue* renderQueue{};
	mutable const FrameBuffer* sceneBuffer{};
	const FrameBuffer* intermediateBuffer{};
	mutable MaterialCache materialCache;

	struct {
		const FrameBuffer* self;

		int32_t positionTextureIdx;
		int32_t normalTextureIdx;
		int32_t albedoTextureIdx;
		int32_t ormTextureIdx;
		int32_t depthTextureIdx;
	} gBuffer{};

	struct {
		const FrameBuffer* buffer;
		int32_t kernelSize;
		int32_t noiseTextureSize;
		float radius;
		float bias;
		float intensity;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		int32_t textureSlot;
	} ssao{};

	struct {
		const std::array<DirectionalLightComponent*, MAX_DIRECTIONAL_LIGHTS>* dirLights;
		const std::array<PointLightComponent*, MAX_POINT_LIGHTS>* pointLights;
		const std::array<SpotLightComponent*, MAX_SPOT_LIGHTS>* spotLights;
		uint32_t maxDirLights, maxPointLights, maxSpotLights;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
	} light{};

	struct {
		const Camera* self;
		const math::Frustum* frustum;
		glm::mat4 skyView;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;
	} camera{};

	struct {
		uint32_t width, height;
	} screen{};

	struct {
		int32_t textureSlot;
		uint32_t width, height;

		struct {
			const UniformBuffer* self;
			uint32_t binding;
			const char* blockName;
		} ubo;

		struct {
			const Shader* shader;
			uint32_t maxLights;
			float height, nearPlane, farPlane, left, right, bottom, top;
		} directional;

		struct {
			const Shader* shader;
			uint32_t maxLights;
			float nearPlane, farPlane, fovy;
		} omnidirectional;

		struct {
			const Shader* shader;
			uint32_t maxLights;
			float nearPlane, farPlane;
		} perspective;
	} shadow{};

	struct {
		struct {
			mutable uint32_t binding;
		} envMap;

		struct {
			const BaseFrameBuffer* self;
			int32_t textureSlot;
		} irradianceMap;

		struct {
			const BaseFrameBuffer* self;
			int32_t textureSlot;
		} prefilterMap;

		struct {
			const BaseFrameBuffer* self;
			int32_t textureSlot;
		} brdfLUT;

		int32_t albedoTextureSlot;
		int32_t normalTextureSlot;
		int32_t roughnessMetallicTextureSlot;
		int32_t aoTextureSlot;
		int32_t emissiveTextureSlot;
		int32_t heightTextureSlot;
	} PBR{};

	RenderContext() = default;

	RenderContext(const RenderContext&) = delete;

	RenderContext& operator=(const RenderContext&) = delete;
};
