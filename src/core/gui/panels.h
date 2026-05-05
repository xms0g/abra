#pragma once
#include <cstdint>
#include <vector>

struct EntityState;
class EventBus;
class Entity;

namespace GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates);

void renderDebugViewsPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates);

void renderLightPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates);

void renderDirLight(const Entity& entity, EventBus& eventBus, EntityState& entityState);

void renderSpotLight(const Entity& entity, EventBus& eventBus, EntityState& entityState);

void renderPointLight(const Entity& entity, EventBus& eventBus, EntityState& entityState);

void renderPostProcessPanel(EventBus& eventBus);
}
