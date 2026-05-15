#pragma once
#include "../../config/config.hpp"
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
	const UniformBuffer& ubo() const;

	std::array<DirectionalLightComponent*, MAX_DIRECTIONAL_LIGHTS>& dirLights();
	std::array<PointLightComponent*, MAX_POINT_LIGHTS>& pointLights();
	std::array<SpotLightComponent*, MAX_SPOT_LIGHTS>& spotLights();

private:
	void updateLightUBO() const;

	void onGuiUpdate(const GuiLightEvent& event);

	EventBus* mEventBus{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::array<DirectionalLightComponent*, MAX_DIRECTIONAL_LIGHTS> mDirLights{};
	std::array<PointLightComponent*, MAX_POINT_LIGHTS> mPointLights{};
	std::array<SpotLightComponent*, MAX_SPOT_LIGHTS> mSpotLights{};
};
