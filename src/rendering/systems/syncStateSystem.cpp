#include "syncStateSystem.h"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../../math/matrix.h"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiLightEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"

SyncStateSystem::~SyncStateSystem() = default;

void SyncStateSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
	mCtx = &ctx;
	mEventBus = &eventBus;
	eventBus.subscribeToEvent<SyncStateSystem, GuiDebugEvent>(this, &SyncStateSystem::onDebugUpdate);
	eventBus.subscribeToEvent<SyncStateSystem, GuiTransformEvent>(this, &SyncStateSystem::onTransformUpdate);
	eventBus.subscribeToEvent<SyncStateSystem, GuiLightEvent>(this, &SyncStateSystem::onLightUpdate);
}

void SyncStateSystem::onDebugUpdate(const GuiDebugEvent& event) {
	uint32_t& mode = mCtx->renderData->entity.debugModes[event.entityID];
	mode = event.mode;
}

void SyncStateSystem::onTransformUpdate(const GuiTransformEvent& event) {
	auto& position = mCtx->renderData->entity.positions[event.entityID];
	auto& rotation = mCtx->renderData->entity.rotations[event.entityID];
	auto& scale = mCtx->renderData->entity.scales[event.entityID];
	auto& model = mCtx->renderData->entity.models[event.entityID];
	auto& normal = mCtx->renderData->entity.normals[event.entityID];

	position = event.position;
	rotation = event.rotation;
	scale = event.scale;
	model = math::modelMatrix(event.position, event.rotation, event.scale);
	normal = math::normalMatrix(model);

	mEventBus->emitEvent<UpdateShadowMapEvent>();
}

void SyncStateSystem::onLightUpdate(const GuiLightEvent& event) {
	auto& color = mCtx->renderData->material.colors[event.matIdx];
	color = event.diffuse;
}
