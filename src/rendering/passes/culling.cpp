#include "culling.h"
#include "../frameGraph.h"
#include "../mesh/mesh.h"
#include "../command.hpp"
#include "../context/renderData.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/registry.h"
#include "../../math/boundingVolume.h"
#include "../../core/camera.h"

CullingPass::CullingPass() = default;

CullingPass::~CullingPass() = default;

void CullingPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mEncoder = GraphicsEncoder{};

	mOpaqueGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("opaque").data(),
		ctx.queueRegistry->get<RenderGroup>("opaque").size());
	mUnlitGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("unlit").data(),
		ctx.queueRegistry->get<RenderGroup>("unlit").size());
	mBlendGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("blend").data(),
		ctx.queueRegistry->get<RenderGroup>("blend").size());
	mDeferredGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("deferred").data(),
		ctx.queueRegistry->get<RenderGroup>("deferred").size());
	mDebugGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("debug").data(),
		ctx.queueRegistry->get<RenderGroup>("debug").size());
	mTerrainGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("terrain").data(),
		ctx.queueRegistry->get<RenderGroup>("terrain").size());
	mSkyboxGroups = std::span(
		ctx.queueRegistry->get<RenderGroup>("skybox").data(),
		ctx.queueRegistry->get<RenderGroup>("skybox").size());
	mOpaqueCommands = &ctx.queueRegistry->get<DrawCommand>("OpaqueCommands");
	mUnlitCommands = &ctx.queueRegistry->get<DrawCommand>("UnlitCommands");
	mBlendCommands = &ctx.queueRegistry->get<DrawCommand>("BlendCommands");
	mDeferredCommands = &ctx.queueRegistry->get<DrawCommand>("DeferredCommands");
	mDebugCommands = &ctx.queueRegistry->get<DrawCommand>("DebugCommands");
	mTerrainCommands = &ctx.queueRegistry->get<DrawCommand>("TerrainCommands");
	mSkyboxCommands = &ctx.queueRegistry->get<DrawCommand>("SkyboxCommands");
}

void CullingPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.clearFrameBuffer(ClearMask::Color | ClearMask::Depth);

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

			const bool isVisible = math::AABB::isMeshInFrustum(frustum, min, max, model);

			if (isVisible) {
				outQueue.push_back({
					.entityID = entityID,
					.debugMode = ctx.renderData->entity.debugModes[entityID],
					.material = {
						.idx = matBatch.materialIndex,
						.flags = ctx.renderData->material.flags[matBatch.materialIndex],
						.color = ctx.renderData->material.colors[matBatch.materialIndex],
						.alphaCutoff = ctx.renderData->material.alphaCutoffs[matBatch.materialIndex],
						.heightScale = ctx.renderData->entity.heightScales[entityID],
						.textures = std::span<const TextureHandle>(
							ctx.renderData->material.textures.data() + matBatch.textureOffset,
							matBatch.textureCount)
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
