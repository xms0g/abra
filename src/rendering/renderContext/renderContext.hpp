#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../material/material.hpp"
#include "../../resource/resourceManager.h"

namespace math {
struct Frustum;
}

class BaseFrameBuffer;
class FrameBuffer;
class UniformBuffer;
class Camera;
struct RenderQueue;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderQueue* renderQueue{};
	mutable const FrameBuffer* sceneBuffer{};
	const FrameBuffer* intermediateBuffer{};
	mutable MaterialCache materialCache;
	const FrameBuffer* gBuffer{};

	struct {
		const FrameBuffer* buffer;
		const UniformBuffer* ubo;
	} ssao{};

	struct {
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
		const UniformBuffer* ubo;
	} light{};

	struct {
		const Camera* self;
		glm::mat4 skyView;
		const UniformBuffer* ubo;
	} camera{};

	struct {
		const UniformBuffer* ubo;
	} shadow{};

	struct {
		struct {
			mutable uint32_t binding;
		} envMap;

		const BaseFrameBuffer* irradianceMap;
		const BaseFrameBuffer* prefilterMap;
		const BaseFrameBuffer* brdfLUT;
	} PBR{};

	RenderContext() = default;

	RenderContext(const RenderContext&) = delete;

	RenderContext& operator=(const RenderContext&) = delete;
};
