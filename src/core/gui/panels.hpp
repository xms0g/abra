#pragma once
#include <cstdint>

struct SpotLightComponent;
struct PointLightComponent;
struct DirectionalLightComponent;
struct TransformComponent;
class EventBus;
class Entity;

namespace GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(TransformComponent& transform, bool& transformChanged);

void renderDebugViewsPanel(const Entity& entity, EventBus& eventBus);

bool renderLightPanel(const Entity& entity,  bool& lightChanged, uint32_t& lightIdx);

void renderDirLight(DirectionalLightComponent& dirlight,  bool& lightChanged, uint32_t& lightIdx);

void renderPointLight(PointLightComponent& pointlight,  bool& lightChanged, uint32_t& lightIdx);

void renderSpotLight(SpotLightComponent& spotlight,  bool& lightChanged, uint32_t& lightIdx);

void renderPostProcessPanel(EventBus& eventBus);
}
