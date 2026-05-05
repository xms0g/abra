#include "syncStatePass.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"

SyncStatePass::~SyncStatePass() = default;

void SyncStatePass::configure(const RenderContext& ctx, EventBus& eventBus) {
	eventBus.subscribeToEvent<SyncStatePass, GuiDebugEvent>(this, &SyncStatePass::onDebugUpdate);
}

void SyncStatePass::execute(const RenderContext& ctx) {
	syncDebugState(ctx);

}

void SyncStatePass::syncDebugState(const RenderContext& ctx) const {
	for (auto& [entity, matBatch]: ctx.renderQueue->debugGroups) {
		if (entity.id != mEntityID)
			continue;

		entity.debugMode = mDebugMode;
	}
}

void SyncStatePass::onDebugUpdate(const GuiDebugEvent& event) {
	mEntityID = event.entityID;
	mDebugMode = event.mode;
}
