#include "culling.h"
#include "../graph.h"
#include "../mesh/mesh.h"
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
	mBlendGroups = &ctx.queueRegistry->get<RenderGroup>("blend");
	mDeferredGroups = &ctx.queueRegistry->get<RenderGroup>("deferred");
	mDebugGroups = &ctx.queueRegistry->get<RenderGroup>("debug");
	mVisibleOpaque = &ctx.queueRegistry->get<VisibleObject>("visibleOpaque");
	mVisibleBlend = &ctx.queueRegistry->get<VisibleObject>("visibleBlend");
	mVisibleDeferred = &ctx.queueRegistry->get<VisibleObject>("visibleDeferred");
	mVisibleDebug = &ctx.queueRegistry->get<VisibleObject>("visibleDebug");
}

void CullingPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
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
	RenderQueue<VisibleObject>& outQueue) {
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
