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

	std::vector<DirectionalLightComponent*>& dirLights();

	std::vector<PointLightComponent*>& pointLights();

	std::vector<SpotLightComponent*>& spotLights();

private:
	void updateLightUBO() const;

	void onGuiUpdate(const GuiLightEvent& event);

	EventBus* mEventBus{};
	const RenderContext* mCtx{};
	std::unique_ptr<UniformBuffer> mUBO;
	std::vector<DirectionalLightComponent*> mDirLights{};
	std::vector<PointLightComponent*> mPointLights{};
	std::vector<SpotLightComponent*> mSpotLights{};
};
