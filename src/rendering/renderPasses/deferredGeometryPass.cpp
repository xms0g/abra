#include "deferredGeometryPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../config/configManager.h"

DeferredGeometryPass::DeferredGeometryPass(const RenderContext& ctx) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("gBuffer");
	mObjects = &ctx.renderQueue->get<std::vector<RenderableObject>>("visibleDeferred");

	const TextureBinding textureBindings[] = {
		{"material.texture_albedo", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.albedo.textureSlot")},
		{"material.texture_normal", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.normal.textureSlot")},
		{"material.texture_roughnessMetallic", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.roughnessMetallic.textureSlot")},
		{"material.texture_ao", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.ao.textureSlot")},
		{"material.texture_emissive", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.emissive.textureSlot")},
		{"material.texture_height", CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.height.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::execute(const RenderContext& ctx, RenderGraph& graph) {
	graph.getResource("gBuffer").bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->activate();

	for (const auto& [entityID, materialIdx, textureOffset, textureCount, meshIdx, shader]: *mObjects) {
		RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *mShader);
		RenderCommand::setupTransform(entityID, ctx, *mShader);

		const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderData->mesh.indexCounts[meshIdx];

		RenderCommand::drawMesh(vao, vertexCount, indexCount);
	}
}
