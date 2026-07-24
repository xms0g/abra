#include "cullingPass.h"
#include "../renderGraph.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../core/camera.h"

CullingPass::CullingPass() = default;

CullingPass::~CullingPass() = default;

void CullingPass::configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) {
	mOpaqueGroups = &ctx.queueRegistry->get<RenderGroup>("opaque");
	mBlendGroups = &ctx.queueRegistry->get<RenderGroup>("blend");
	mDeferredGroups = &ctx.queueRegistry->get<RenderGroup>("deferred");
	mDebugGroups = &ctx.queueRegistry->get<RenderGroup>("debug");
	mVisibleOpaque = &ctx.queueRegistry->get<RenderableObject>("visibleOpaque");
	mVisibleBlend = &ctx.queueRegistry->get<RenderableObject>("visibleBlend");
	mVisibleDeferred = &ctx.queueRegistry->get<RenderableObject>("visibleDeferred");
	mVisibleDebug = &ctx.queueRegistry->get<RenderableObject>("visibleDebug");
}

void CullingPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	const auto frustum = ctx.camera->generateFrustum();

	cullScene(ctx, frustum, *mOpaqueGroups, *mVisibleOpaque);
	cullScene(ctx, frustum, *mDeferredGroups, *mVisibleDeferred);
	cullScene(ctx, frustum, *mBlendGroups, *mVisibleBlend);
	cullScene(ctx, frustum, *mDebugGroups, *mVisibleDebug);
}

void CullingPass::cullScene(
	const RenderContext& ctx,
	const math::Frustum& frustum,
	const RenderQueue<RenderGroup>& groups,
	RenderQueue<RenderableObject>& outQueue) {
	outQueue.clear();

	for (const auto& [entityID, matBatch]: groups) {
		const auto& model = ctx.renderData->entity.models[entityID];
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
					.materialIndex = matBatch.materialIndex,
					.textureOffset = matBatch.textureOffset,
					.textureCount = matBatch.textureCount,
					.meshIndex = meshIdx,
					.shader = matBatch.shader
				});
			}
		}
	}

	std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
		return a.materialIndex < b.materialIndex;
	});
}
