#pragma once
#include <cstdint>
#include <vector>

struct EntityState;
class EventBus;
class Entity;

namespace  GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates);

void renderDebugViewsPanel(const Entity& entity, EventBus& eventBus, std::vector<EntityState>& entityStates);

void renderLightPanel(const Entity& entity);

void renderDirLight(const Entity& entity);

void renderSpotLight(const Entity& entity);

void renderPointLight(const Entity& entity);

void renderPostProcessPanel(EventBus& eventBus);

}
