#pragma once
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../event/events/guiLightEvent.hpp"

template<typename T>
void LightSystem::processLight(const Entity& entity, const GuiLightEvent& e, std::vector<T*>& lightList) {
	if (const auto& light = entity.tryGetComponent<T>()) {
		mCtx->renderData->material.colors[e.matIdx] = light->diffuse;
		auto& transform = entity.getComponent<TransformComponent>();
		light->position = transform.position;
		lightList[e.lightIdx] = light;
	}
}
