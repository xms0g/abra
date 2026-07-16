#include "cullingPass.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../core/camera.h"

CullingPass::~CullingPass() = default;

void CullingPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mOpaqueGroups = &ctx.renderQueue->get<std::vector<RenderGroup> >("opaque");
	mBlendGroups = &ctx.renderQueue->get<std::vector<RenderGroup> >("blend");
	mDeferredGroups = &ctx.renderQueue->get<std::vector<RenderGroup> >("deferred");
	mDebugGroups = &ctx.renderQueue->get<std::vector<RenderGroup> >("debug");
	mVisibleOpaque = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleOpaque");
	mVisibleBlend = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleBlend");
	mVisibleDeferred = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleDeferred");
	mVisibleDebug = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleDebug");
}

void CullingPass::execute(const RenderContext& ctx) {
	const auto frustum = ctx.camera.self->generateFrustum();

	cullScene(ctx, frustum, *mOpaqueGroups, *mVisibleOpaque);
	cullScene(ctx, frustum, *mDeferredGroups, *mVisibleDeferred);
	cullScene(ctx, frustum, *mBlendGroups, *mVisibleBlend);
	cullScene(ctx, frustum, *mDebugGroups, *mVisibleDebug);
}

void CullingPass::cullScene(
	const RenderContext& ctx,
	const math::Frustum& frustum,
	const std::vector<RenderGroup>& groups,
	std::vector<RenderableObject>& outQueue) {
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
					entityID,
					matBatch.materialIndex,
					matBatch.textureOffset,
					matBatch.textureCount,
					meshIdx,
					matBatch.shader
				});
			}
		}
	}

	std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
		return a.materialIndex < b.materialIndex;
	});
}
