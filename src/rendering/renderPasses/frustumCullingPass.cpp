#include "frustumCullingPass.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"

FrustumCullingPass::~FrustumCullingPass() = default;

void FrustumCullingPass::configure(const RenderContext& ctx, EventBus& eventBus) {
}

void FrustumCullingPass::execute(const RenderContext& ctx) {
	auto cullItems = [&](const std::vector<RenderGroup>& groups, std::vector<RenderableObject>& outQueue) -> void {
		outQueue.clear();

		for (const auto& [entityID, matBatch]: groups) {
			auto& [ePos, eRot, eScale, eModel, eNormal] = ctx.renderQueue->entityTransforms[entityID];
			auto& [center, extents] = ctx.renderQueue->entityBVs[entityID];

			if (!math::AABB::isOnFrustum(*ctx.camera.frustum, eModel, center, extents)) {
				continue;
			}

			const auto& [matIdx, shader, meshes] = matBatch;
			for (const auto& meshIdx: meshes) {
				const glm::vec3& max = ctx.renderQueue->meshMaxCounts[meshIdx];
				const glm::vec3& min = ctx.renderQueue->meshMinCounts[meshIdx];

				const bool isVisible = math::AABB::isMeshInFrustum(*ctx.camera.frustum, min, max, eModel);

				if (isVisible) {
					outQueue.push_back({entityID, eModel, eNormal, matIdx, meshIdx, shader});
				}
			}
		}

		std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
			return a.materialIndex < b.materialIndex;
		});
	};

	cullItems(ctx.renderQueue->opaqueGroups, ctx.renderQueue->opaqueObjects);
	cullItems(ctx.renderQueue->deferredGroups, ctx.renderQueue->deferredObjects);
	cullItems(ctx.renderQueue->blendGroups, ctx.renderQueue->blendObjects);
	cullItems(ctx.renderQueue->debugGroups, ctx.renderQueue->dbgObjects);
	cullItems(ctx.renderQueue->shadowGroups, ctx.renderQueue->shadowedObjects);
}
