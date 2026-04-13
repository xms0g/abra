#include "frustumCullingPass.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../math/boundingVolume.h"

FrustumCullingPass::~FrustumCullingPass() = default;

void FrustumCullingPass::configure(const RenderContext& ctx) {
}

void FrustumCullingPass::execute(const RenderContext& ctx) {
	auto cullItems = [&](const std::vector<RenderGroup>& groups, std::vector<RenderableObject>& outQueue) -> void {
		outQueue.clear();

		for (const auto& [entity, matBatch]: groups) {
			if (!entity.bv->isOnFrustum(*ctx.camera.frustum,
			                            entity.transform->position,
			                            entity.transform->rotation,
			                            entity.transform->scale)) {
				continue;
			}

			const auto& [material, shader, meshes] = matBatch;
			for (const auto& mesh: *meshes) {
				const bool isVisible = entity.bv->isMeshInFrustum(
					*ctx.camera.frustum,
					mesh.min(), mesh.max(),
					entity.transform->position,
					entity.transform->rotation,
					entity.transform->scale);

				if (isVisible) {
					outQueue.push_back({&entity, material, shader, &mesh});
				}
			}
		}
	};

	cullItems(ctx.renderQueue->opaqueGroups, ctx.renderQueue->opaqueObjects);
	cullItems(ctx.renderQueue->deferredGroups, ctx.renderQueue->deferredObjects);
	cullItems(ctx.renderQueue->blendGroups, ctx.renderQueue->blendObjects);
	cullItems(ctx.renderQueue->shadowGroups, ctx.renderQueue->shadowingObjects);
	cullItems(ctx.renderQueue->debugGroups, ctx.renderQueue->dbgObjects);
}
