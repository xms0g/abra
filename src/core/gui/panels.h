#pragma once
#include <cstdint>
#include <vector>

class EventBus;
class Entity;

namespace GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(const Entity& entity, EventBus& eventBus);

void renderDebugViewsPanel(const Entity& entity, EventBus& eventBus);

void renderLightPanel(const Entity& entity, EventBus& eventBus);

void renderDirLight(const Entity& entity, EventBus& eventBus);

void renderPointLight(const Entity& entity, EventBus& eventBus);

void renderSpotLight(const Entity& entity, EventBus& eventBus);

void renderPostProcessPanel(EventBus& eventBus);
}
