#include "syncStatePass.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"

SyncStatePass::~SyncStatePass() = default;

void SyncStatePass::configure(const RenderContext& ctx, EventBus& eventBus) {
	eventBus.subscribeToEvent<SyncStatePass, GuiDebugEvent>(this, &SyncStatePass::onDebugUpdate);
	eventBus.subscribeToEvent<SyncStatePass, GuiTransformEvent>(this, &SyncStatePass::onTransformUpdate);
}

void SyncStatePass::execute(const RenderContext& ctx) {
	syncDebugState(ctx);
	syncTransformState(ctx);
}

void SyncStatePass::syncDebugState(const RenderContext& ctx) const {
	for (auto& [entityID, matBatch]: ctx.renderQueue->debugGroups) {
		if (entityID != mEntityID)
			continue;

		uint32_t& mode = ctx.renderQueue->entityDebugModes.at(entityID);
		mode = mDebugMode;
	}
}

void SyncStatePass::syncTransformState(const RenderContext& ctx) const {
	for (auto& [entityID, matBatch]: ctx.renderQueue->deferredGroups) {
		if (entityID != mEntityID)
			continue;

		auto& [position, rotation, scale] = ctx.renderQueue->entityTransforms.at(entityID);
		position = transform.position;
		rotation = transform.rotation;
		scale = transform.scale;
		break;
	}
}

void SyncStatePass::onDebugUpdate(const GuiDebugEvent& event) {
	mEntityID = event.entityID;
	mDebugMode = event.mode;
}

void SyncStatePass::onTransformUpdate(const GuiTransformEvent& event) {
	mEntityID = event.entityID;
	transform.position = event.position;
	transform.rotation = event.rotation;
	transform.scale = event.scale;
}
