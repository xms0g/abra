#pragma once
#include <vector>

class IPostEffect;
class Entity;

namespace  GuiPanels {
void renderGraphicsInfoPanel(uint32_t fps);

void renderTransformPanel(const Entity& entity);

void renderDebugViewsPanel(const Entity& entity);

void renderLightPanel(const Entity& entity);

void renderDirLight(const Entity& entity);

void renderSpotLight(const Entity& entity);

void renderPointLight(const Entity& entity);

void renderPostProcessPanel(const std::vector<std::unique_ptr<IPostEffect>>& effects);

}
