#include "culling.h"
#include "../graph.h"
#include "../mesh/mesh.h"
#include "../command.hpp"
#include "../context/renderData.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/visibleObject.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../core/camera.h"

CullingPass::CullingPass() = default;

CullingPass::~CullingPass() = default;

void CullingPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mOpaqueGroups = &ctx.queueRegistry->get<RenderGroup>("opaque");
	mUnlitGroups = &ctx.queueRegistry->get<RenderGroup>("unlit");
	mBlendGroups = &ctx.queueRegistry->get<RenderGroup>("blend");
	mDeferredGroups = &ctx.queueRegistry->get<RenderGroup>("deferred");
	mDebugGroups = &ctx.queueRegistry->get<RenderGroup>("debug");
	mOpaqueCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
	mUnlitCommands = &ctx.queueRegistry->get<DrawCommand>("UnlitCommands");
	mBlendCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
	mDeferredCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
	mDebugCommands = &ctx.queueRegistry->get<DrawCommand>("DebugCommands");
}

void CullingPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto frustum = ctx.camera->generateFrustum();

	cullScene(ctx, frustum, *mOpaqueGroups, *mOpaqueCommands);
	cullScene(ctx, frustum, *mUnlitGroups, *mUnlitCommands);
	cullScene(ctx, frustum, *mDeferredGroups, *mDeferredCommands);
	cullScene(ctx, frustum, *mBlendGroups, *mBlendCommands);
	cullScene(ctx, frustum, *mDebugGroups, *mDebugCommands);
}

void CullingPass::cullScene(
	const RenderContext& ctx,
	const math::Frustum& frustum,
	const RenderQueue<RenderGroup>& groups,
	RenderQueue<DrawCommand>& outQueue) {
	outQueue.clear();

	for (const auto& [entityID, matBatch]: groups) {
		const auto& model = ctx.renderData->entity.models[entityID];
		const auto& normal = ctx.renderData->entity.normals[entityID];
		const auto& center = ctx.renderData->entity.centers[entityID];
		const auto& extents = ctx.renderData->entity.extents[entityID];

		if (!math::AABB::isOnFrustum(frustum, model, center, extents)) {
			continue;
		}

		for (const auto& meshIdx: matBatch.meshIndices) {
			const glm::vec3& max = ctx.renderData->mesh.maxCounts[meshIdx];
			const glm::vec3& min = ctx.renderData->mesh.minCounts[meshIdx];

			const bool isVisible = math::AABB::isMeshInFrustum(frustum, min, max, model);

			if (isVisible) {
				outQueue.push_back({
					.entityID = entityID,
					.debugMode = ctx.renderData->entity.debugModes[entityID],
					.material = {
						.idx = matBatch.materialIndex,
						.flags = ctx.renderData->material.flags[matBatch.materialIndex],
						.textureTarget = ctx.renderData->material.textureTargets[matBatch.materialIndex],
						.color = ctx.renderData->material.colors[matBatch.materialIndex],
						.alphaCutoff = ctx.renderData->material.alphaCutoffs[matBatch.materialIndex],
						.heightScale = ctx.renderData->entity.heightScales[entityID],
						.textures = std::span<const uint32_t>(
							ctx.renderData->material.textures.data() + matBatch.textureOffset,
							matBatch.textureCount)
					},
					.transform = {
						.model = model,
						.normal = normal,
					},
					.mesh = {
						.vao = ctx.renderData->mesh.vaos[meshIdx],
						.vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx],
						.indexCount = ctx.renderData->mesh.indexCounts[meshIdx]
					}
				});
			}
		}
	}

	std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
		return a.material.idx < b.material.idx;
	});
}
