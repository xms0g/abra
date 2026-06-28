#include "cullingPass.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../core/camera.h"

CullingPass::~CullingPass() = default;

void CullingPass::configure(RenderContext& ctx, EventBus& eventBus) {
}

void CullingPass::execute(const RenderContext& ctx) {
	const auto frustum = ctx.camera.self->generateFrustum();

	cullScene(ctx, frustum, ctx.renderData->opaqueGroups, ctx.renderData->opaqueObjects);
	cullScene(ctx, frustum, ctx.renderData->deferredGroups, ctx.renderData->deferredObjects);
	cullScene(ctx, frustum, ctx.renderData->blendGroups, ctx.renderData->blendObjects);
	cullScene(ctx, frustum, ctx.renderData->debugGroups, ctx.renderData->dbgObjects);
}

void CullingPass::cullScene(
	const RenderContext& ctx,
	const math::Frustum& frustum,
	const std::vector<RenderGroup>& groups,
	std::vector<RenderableObject>& outQueue) {
	outQueue.clear();

	for (const auto& [entityID, matBatch]: groups) {
		const auto& model = ctx.renderData->entity.models[entityID];
		const auto& center= ctx.renderData->entity.centers[entityID];
		const auto& extents = ctx.renderData->entity.extents[entityID];

		if (!math::AABB::isOnFrustum(frustum, model, center, extents)) {
			continue;
		}

		const auto& [matIdx, textureOffset, textureCount, shader, meshes] = matBatch;
		for (const auto& meshIdx: meshes) {
			const glm::vec3& max = ctx.renderData->mesh.maxCounts[meshIdx];
			const glm::vec3& min = ctx.renderData->mesh.minCounts[meshIdx];

			const bool isVisible = math::AABB::isMeshInFrustum(frustum, min, max, model);

			if (isVisible) {
				outQueue.push_back({entityID, matIdx, textureOffset, textureCount, meshIdx, shader});
			}
		}
	}

	std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
		return a.materialIndex < b.materialIndex;
	});
}
