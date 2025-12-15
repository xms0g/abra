#include "frustumCullingPass.h"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderCommand.hpp"
#include "../mesh/mesh.h"
#include "../../ECS/registry.h"
#include "../../ECS/components/transform.hpp"
#include "../../math/boundingVolume.h"

FrustumCullingPass::~FrustumCullingPass() = default;

void FrustumCullingPass::configure(const RenderContext& ctx) {
}

void FrustumCullingPass::execute(const RenderContext& ctx) {
	ctx.renderQueue->deferredCommands.clear();
	ctx.renderQueue->forwardCommands.clear();
	ctx.renderQueue->blendCommands.clear();
	ctx.renderQueue->shadowCommands.clear();
	ctx.renderQueue->dbgCommands.clear();

	auto cullItems = [&](const std::vector<RenderGroup>& groups, std::vector<RenderCommand>& outQueue) {
		for (const auto& [entity, matBatch]: groups) {
			if (!entity.bv->isOnFrustum(*ctx.camera.frustum, entity.transform->position, entity.transform->rotation,
			                            entity.transform->scale))
				continue;

			for (auto& [material, shader, meshes] = matBatch; const auto& mesh: *meshes) {
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

	cullItems(ctx.renderQueue->forwardOpaqueGroups, ctx.renderQueue->forwardCommands);
	cullItems(ctx.renderQueue->deferredGroups, ctx.renderQueue->deferredCommands);
	cullItems(ctx.renderQueue->blendGroups, ctx.renderQueue->blendCommands);
	cullItems(ctx.renderQueue->shadowGroups, ctx.renderQueue->shadowCommands);
	cullItems(ctx.renderQueue->debugGroups, ctx.renderQueue->dbgCommands);
}
