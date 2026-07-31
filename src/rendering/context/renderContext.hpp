#pragma once
#include <span>

class FrameBuffer;
class QueueRegistry;
class Camera;
struct PBRBuffers;
struct RenderData;
struct DirectionalLightComponent;
struct PointLightComponent;
struct SpotLightComponent;

struct RenderContext {
	RenderData* renderData{};
	QueueRegistry* queueRegistry{};
	PBRBuffers* pbrBuffers{};

	struct {
		std::span<DirectionalLightComponent*> dirLights;
		std::span<PointLightComponent*> pointLights;
		std::span<SpotLightComponent*> spotLights;
	} light{};

	const Camera* camera{};

	RenderContext() = default;

	RenderContext(const RenderContext&) = delete;

	RenderContext& operator=(const RenderContext&) = delete;
};
