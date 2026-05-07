#pragma once
#include "../ECS/system.hpp"

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

	[[nodiscard]]
	const std::vector<PointLightComponent*>& pointLights() const;

	[[nodiscard]]
	const std::vector<DirectionalLightComponent*>& dirLights() const;

	[[nodiscard]]
	const std::vector<SpotLightComponent*>& spotLights() const;

private:
	void updateLightUBO() const;

	void onGuiUpdate(const GuiLightEvent& event);

	EventBus* mEventBus;
	std::unique_ptr<UniformBuffer> mUBO;
	std::vector<DirectionalLightComponent*> mDirLights;
	std::vector<PointLightComponent*> mPointLights;
	std::vector<SpotLightComponent*> mSpotLights;
};
