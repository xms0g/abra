#include "frustumCullingPass.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../mesh/mesh.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/bv.hpp"
#include "../../ECS/entity.hpp"
#include "../../ECS/components/transform.hpp"
#include "../../math/boundingVolume.h"

FrustumCullingPass::~FrustumCullingPass() = default;

void FrustumCullingPass::configure(const RenderContext& ctx) {
}

void FrustumCullingPass::execute(const RenderContext& ctx) {
	auto cullItems = [&](const std::vector<RenderGroup>& groups) {
		for (const auto& [entity, matBatch]: groups) {
			auto& bvc = entity->getComponent<BoundingVolumeComponent>();
			const auto& tc = entity->getComponent<TransformComponent>();
			const auto& aabb = bvc.bv;

			bvc.isVisible = aabb->isOnFrustum(*ctx.camera.frustum, tc.position, tc.rotation, tc.scale);
			if (!bvc.isVisible) return;

			for (auto& [material, shader, meshes]: matBatch) {
				for (auto& mesh: *meshes) {
					mesh.setVisible(aabb->isMeshInFrustum(*ctx.camera.frustum, mesh.min(), mesh.max(),
														  tc.position, tc.rotation, tc.scale));
				}
			}
		}
	};

	cullItems(ctx.renderQueue->forwardOpaqueGroups);
	cullItems(ctx.renderQueue->deferredGroups);
	cullItems(ctx.renderQueue->blendGroups);
}
