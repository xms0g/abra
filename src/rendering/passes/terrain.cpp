#include "terrain.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../../rendering/shader.h"

TerrainPass::TerrainPass() = default;

TerrainPass::~TerrainPass() = default;

void TerrainPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("terrain");
	mObjects = &ctx.queueRegistry->get<RenderGroup>("terrain");

	constexpr TextureBinding textureBindings[] = {
		{.name = "heightMap", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void TerrainPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();
	const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];

	mShader->bind();
	RenderCommand::setupTransform(entityID, ctx, *mShader);
	RenderCommand::setupMaterial(
		entityID,
		matBatch.materialIndex,
		matBatch.textureOffset,
		matBatch.textureCount,
		ctx,
		*mShader);

	const size_t count = ctx.renderData->mesh.vertexCounts[meshIdx];

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	RenderCommand::drawPatch(vao, static_cast<int32_t>(count));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
