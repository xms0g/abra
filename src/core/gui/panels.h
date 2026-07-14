#pragma once
#include <cstdint>
#include <vector>

struct SpotLightComponent;
struct PointLightComponent;
struct DirectionalLightComponent;
class EventBus;
class Entity;

namespace GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(const Entity& entity, EventBus& eventBus);

void renderDebugViewsPanel(const Entity& entity, EventBus& eventBus);

void renderLightPanel(const Entity& entity, EventBus& eventBus);

void renderDirLight(const Entity& entity, DirectionalLightComponent& dirlight, EventBus& eventBus);

void renderPointLight(const Entity& entity, PointLightComponent& pointlight, EventBus& eventBus);

void renderSpotLight(const Entity& entity, SpotLightComponent& spotlight, EventBus& eventBus);

void renderPostProcessPanel(EventBus& eventBus);
}
