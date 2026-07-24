#include "debugPass.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/uniformBuffer.h"
#include "../buffers/frameBuffer.h"
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../context/visibleObject.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/components/debug.hpp"

DebugPass::DebugPass() = default;

DebugPass::~DebugPass() = default;

void DebugPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mObjects = &ctx.queueRegistry->get<VisibleObject>("visibleDebug");

	mDebugShaders = {
		nullptr,
		RESOURCE_MANAGER_INSTANCE.get<Shader>("debugNormal"),
		RESOURCE_MANAGER_INSTANCE.get<Shader>("debugWireframe")
	};
}

void DebugPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	for (const auto& object: *mObjects) {
		const uint32_t mode = ctx.renderData->entity.debugModes[object.entityID];

		if (mode == None)
			continue;

		const auto& dbgShader = mDebugShaders[mode];
		dbgShader->bind();

		RenderCommand::setupTransform(object.entityID, ctx, *dbgShader);

		const uint32_t vao = ctx.renderData->mesh.vaos[object.meshIndex];
		const size_t vertexCount = ctx.renderData->mesh.vertexCounts[object.meshIndex];
		const size_t indexCount = ctx.renderData->mesh.indexCounts[object.meshIndex];

		RenderCommand::drawMesh(vao, vertexCount, indexCount);
	}
}
