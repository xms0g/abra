#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../material/material.hpp"
#include "../../resource/resourceManager.h"

class QueueRegistry;
class Camera;
struct RenderData;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderData* renderData{};
	QueueRegistry* queueRegistry{};
	mutable MaterialCache materialCache;

	struct {
		const std::vector<DirectionalLightComponent*>* dirLights;
		const std::vector<PointLightComponent*>* pointLights;
		const std::vector<SpotLightComponent*>* spotLights;
	} light{};

	const Camera* camera{};

	RenderContext() = default;

	RenderContext(const RenderContext&) = delete;

	RenderContext& operator=(const RenderContext&) = delete;
};
