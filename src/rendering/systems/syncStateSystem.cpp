#include "syncStateSystem.hpp"
#include "../context/renderData.hpp"
#include "../context/renderContext.hpp"
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
	mCtx->renderData->updateDebugMode(event.entityID, event.mode);
}

void SyncStateSystem::onTransformUpdate(const GuiTransformEvent& event) {
	mCtx->renderData->updateTransform(event.entityID, event.position, event.rotation, event.scale);
	mEventBus->emitEvent<UpdateShadowMapEvent>();
}
