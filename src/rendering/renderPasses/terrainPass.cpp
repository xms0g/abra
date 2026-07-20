#include "terrainPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../rendering/shader.h"

TerrainPass::TerrainPass(const RenderContext& ctx) {
	mWrites = {"sceneBuffer"};
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("terrain");
	mObjects = &ctx.renderQueue->get<std::vector<RenderGroup> >("terrain");

	constexpr TextureBinding textureBindings[] = {
		{.name = "heightMap", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

TerrainPass::~TerrainPass() = default;

void TerrainPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();
	const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];

	mShader->activate();
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
