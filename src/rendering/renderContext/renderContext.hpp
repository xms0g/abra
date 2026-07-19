#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../material/material.hpp"
#include "../../resource/resourceManager.h"

namespace math {
struct Frustum;
}

class RenderQueue;
class FrameBuffer;
class UniformBuffer;
class Camera;
struct RenderData;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderData* renderData{};
	RenderQueue* renderQueue{};
	mutable MaterialCache materialCache;

	struct {
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
	} light{};

	struct {
		const Camera* self;
		glm::mat4 skyView;
	} camera{};

	RenderContext() = default;

	RenderContext(const RenderContext&) = delete;

	RenderContext& operator=(const RenderContext&) = delete;
};
