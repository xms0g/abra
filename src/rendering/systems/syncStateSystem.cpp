#include "syncStateSystem.h"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../../math/matrix.h"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"

SyncStateSystem::~SyncStateSystem() = default;

void SyncStateSystem::configure(const RenderContext& ctx, EventBus& eventBus) {
	mCtx = &ctx;
	mEventBus = &eventBus;
	eventBus.subscribeToEvent<SyncStateSystem, GuiDebugEvent>(this, &SyncStateSystem::onDebugUpdate);
	eventBus.subscribeToEvent<SyncStateSystem, GuiTransformEvent>(this, &SyncStateSystem::onTransformUpdate);
}

void SyncStateSystem::onDebugUpdate(const GuiDebugEvent& event) {
	mCtx->renderData->entity.debugModes[event.entityID] = event.mode;
}

void SyncStateSystem::onTransformUpdate(const GuiTransformEvent& event) {
	mCtx->renderData->entity.positions[event.entityID] = event.position;
	mCtx->renderData->entity.rotations[event.entityID] = event.rotation;
	mCtx->renderData->entity.scales[event.entityID] = event.scale;

	const auto model = math::modelMatrix(event.position, event.rotation, event.scale);
	mCtx->renderData->entity.models[event.entityID] = model;
	mCtx->renderData->entity.normals[event.entityID] = math::normalMatrix(model);

	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
