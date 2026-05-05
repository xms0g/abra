#include "frustumCullingPass.h"
#include "../mesh/mesh.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../math/matrix.h"

FrustumCullingPass::~FrustumCullingPass() = default;

void FrustumCullingPass::configure(const RenderContext& ctx, EventBus& eventBus) {
}

void FrustumCullingPass::execute(const RenderContext& ctx) {
	auto cullItems = [&](const std::vector<RenderGroup>& groups, std::vector<RenderableObject>& outQueue) -> void {
		outQueue.clear();

		for (const auto& [entityID, matBatch]: groups) {
			auto& [position, rotation, scale] = ctx.renderQueue->entityTransforms.at(entityID);
			auto& [center, extents] = ctx.renderQueue->entityBVs.at(entityID);

			const glm::mat4 model = math::modelMatrix(position, rotation, scale);

			if (!math::AABB::isOnFrustum(*ctx.camera.frustum, model, center, extents)) {
				continue;
			}

			const auto& [material, shader, meshes] = matBatch;
			for (const auto& mesh: *meshes) {
				const bool isVisible = math::AABB::isMeshInFrustum(*ctx.camera.frustum, mesh.min(), mesh.max(), model);

				if (isVisible) {
					outQueue.push_back({entityID, material, shader, &mesh});
				}
			}
		}

		std::ranges::sort(outQueue, [](const auto& a, const auto& b) {
			return a.material->id < b.material->id;
		});
	};

	cullItems(ctx.renderQueue->opaqueGroups, ctx.renderQueue->opaqueObjects);
	cullItems(ctx.renderQueue->deferredGroups, ctx.renderQueue->deferredObjects);
	cullItems(ctx.renderQueue->blendGroups, ctx.renderQueue->blendObjects);
	cullItems(ctx.renderQueue->debugGroups, ctx.renderQueue->dbgObjects);
}
