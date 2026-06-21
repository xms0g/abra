#pragma once
#include "../../ECS/system.hpp"

struct GuiLightEvent;
class EventBus;
struct RenderContext;
class UniformBuffer;
struct PointLightComponent;
struct DirectionalLightComponent;
struct SpotLightComponent;

class LightSystem final : public System {
public:
	LightSystem();

	void configure(const RenderContext& ctx, EventBus& eventBus);

	[[nodiscard]]
	const UniformBuffer* ubo() const;

	std::array<DirectionalLightComponent*, 1>& dirLights();

	std::array<PointLightComponent*, 4>& pointLights();

	std::array<SpotLightComponent*, 4>& spotLights();

private:
	void updateLightUBO() const;

	void onGuiUpdate(const GuiLightEvent& event);

	EventBus* mEventBus{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::array<DirectionalLightComponent*, 1> mDirLights{};
	std::array<PointLightComponent*, 4> mPointLights{};
	std::array<SpotLightComponent*, 4> mSpotLights{};
};
