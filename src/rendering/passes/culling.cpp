#include "culling.hpp"
#include <algorithm>
#include "../frameGraph.hpp"
#include "../command.hpp"
#include "../graphicsEncoder.hpp"
#include "../context/renderData.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/registry.hpp"
#include "../../math/boundingVolume.hpp"
#include "../../core/camera.hpp"

CullingPass::CullingPass() = default;

CullingPass::~CullingPass() = default;

void CullingPass::configure(const RenderContext& ctx,
                            const FrameGraph& graph,
                            GraphicsEncoder& encoder,
                            EventBus& eventBus) {
	mOpaqueGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("opaque").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("opaque").size());
	mUnlitGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("unlit").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("unlit").size());
	mBlendGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("blend").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("blend").size());
	mDeferredGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("deferred").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("deferred").size());
	mDebugGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("debug").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("debug").size());
	mTerrainGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("terrain").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("terrain").size());
	mSkyboxGroups = std::span(
		ctx.queueRegistry->fetchQueue<RenderGroup>("skybox").data(),
		ctx.queueRegistry->fetchQueue<RenderGroup>("skybox").size());

	mOpaqueCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("OpaqueCommands");
	mUnlitCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("UnlitCommands");
	mBlendCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("BlendCommands");
	mDeferredCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("DeferredCommands");
	mDebugCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("DebugCommands");
	mTerrainCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("TerrainCommands");
	mSkyboxCommands = &ctx.queueRegistry->fetchQueue<DrawCommand>("SkyboxCommands");
}

void CullingPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto frustum = ctx.camera->generateFrustum();

	cullScene(ctx, frustum, mOpaqueGroups, *mOpaqueCommands);
	cullScene(ctx, frustum, mUnlitGroups, *mUnlitCommands);
	cullScene(ctx, frustum, mDeferredGroups, *mDeferredCommands);
	cullScene(ctx, frustum, mBlendGroups, *mBlendCommands);
	cullScene(ctx, frustum, mDebugGroups, *mDebugCommands);
	cullScene(ctx, frustum, mTerrainGroups, *mTerrainCommands);
	cullScene(ctx, frustum, mSkyboxGroups, *mSkyboxCommands);
}

void CullingPass::cullScene(
	const RenderContext& ctx,
	const math::Frustum& frustum,
	std::span<RenderGroup> groups,
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

			if (math::AABB::isMeshInFrustum(frustum, min, max, model)) {
				outQueue.push_back({
					.entityID = entityID,
					.debugMode = ctx.renderData->entity.debugModes[entityID],
					.material = {
						.idx = matBatch.materialIndex,
						.flags = ctx.renderData->material.flags[matBatch.materialIndex],
						.color = ctx.renderData->material.colors[matBatch.materialIndex],
						.alphaCutoff = ctx.renderData->material.alphaCutoffs[matBatch.materialIndex],
						.heightScale = ctx.renderData->entity.heightScales[entityID]
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
