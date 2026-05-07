#include "syncStatePass.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../../math/matrix.h"
#include "../../event/eventBus.hpp"
#include "../../event/events/guiDebugEvent.hpp"
#include "../../event/events/guiTransformEvent.hpp"
#include "../../event/events/updateShadowMapEvent.hpp"

SyncStatePass::~SyncStatePass() = default;

void SyncStatePass::configure(const RenderContext& ctx, EventBus& eventBus) {
	mEventBus = &eventBus;
	eventBus.subscribeToEvent<SyncStatePass, GuiDebugEvent>(this, &SyncStatePass::onDebugUpdate);
	eventBus.subscribeToEvent<SyncStatePass, GuiTransformEvent>(this, &SyncStatePass::onTransformUpdate);
}

void SyncStatePass::execute(const RenderContext& ctx) {
	syncDebugState(ctx);
	syncTransformState(ctx);
}

void SyncStatePass::syncDebugState(const RenderContext& ctx) {
	if (!debug.isDirty)
		return;

	uint32_t& mode = ctx.renderQueue->entity.debugModes[mEntityID];
	mode = debug.mode;
	debug.isDirty = false;
}

void SyncStatePass::syncTransformState(const RenderContext& ctx) {
	if (!transform.isDirty)
		return;

	auto& position = ctx.renderQueue->entity.positions[mEntityID];
	auto& rotation = ctx.renderQueue->entity.rotations[mEntityID];
	auto& scale = ctx.renderQueue->entity.scales[mEntityID];
	auto& model = ctx.renderQueue->entity.models[mEntityID];
	auto& normal = ctx.renderQueue->entity.normals[mEntityID];

	position = transform.position;
	rotation = transform.rotation;
	scale = transform.scale;
	model = transform.model;
	normal = transform.normal;
	transform.isDirty = false;

	mEventBus->emitEvent<UpdateShadowMapEvent>();
}

void SyncStatePass::onDebugUpdate(const GuiDebugEvent& event) {
	debug.isDirty = true;
	mEntityID = event.entityID;
	debug.mode = event.mode;
}

void SyncStatePass::onTransformUpdate(const GuiTransformEvent& event) {
	transform.isDirty = true;
	mEntityID = event.entityID;
	transform.position = event.position;
	transform.rotation = event.rotation;
	transform.scale = event.scale;
	transform.model = math::modelMatrix(event.position, event.rotation, event.scale);
	transform.normal = math::normalMatrix(transform.model);
}
