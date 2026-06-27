#include "terrainPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../rendering/shader.h"

TerrainPass::~TerrainPass() = default;

void TerrainPass::configure(RenderContext& ctx, EventBus& eventBus) {
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	const auto& [entity, matb] = ctx.renderQueue->terrain.front();

	constexpr TextureBinding textureBindings[] = {
		{"heightMap", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *matb.shader);
	RenderCommand::bindShadowMaps(ctx);
}

void TerrainPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	const auto& [entityID, matb] = ctx.renderQueue->terrain.front();
	const auto [materialIdx, textureOffset, textureCount, shader, meshes] = matb;
	const uint32_t meshIdx = meshes.front();
	const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];

	shader->activate();
	RenderCommand::setupTransform(entityID, ctx, *shader);
	RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *shader);

	const size_t count = ctx.renderQueue->mesh.vertexCounts[meshIdx];

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	RenderCommand::drawPatch(vao, count);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
