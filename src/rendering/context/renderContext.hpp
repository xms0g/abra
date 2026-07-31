#pragma once
#include <span>
#include <memory>
#include <unordered_map>

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
