#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "../material/material.hpp"

class FrameBuffer;
class QueueRegistry;
class Camera;
struct RenderData;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderData* renderData{};
	QueueRegistry* queueRegistry{};
	std::unordered_map<std::string, std::unique_ptr<FrameBuffer> >* pbrBuffers{};
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
