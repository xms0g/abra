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
	mInputs = {"visibleDeferred"};
	mOutputs = {"gBuffer"};
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("gBuffer");
	mObjects = &ctx.renderQueue->get<std::vector<RenderableObject>>("visibleDeferred");

	const TextureBinding textureBindings[] = {
		{.name = "material.texture_albedo", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.albedo.textureSlot")},
		{.name = "material.texture_normal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.normal.textureSlot")},
		{.name = "material.texture_roughnessMetallic", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.roughnessMetallic.textureSlot")},
		{.name = "material.texture_ao", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.ao.textureSlot")},
		{.name = "material.texture_emissive", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.emissive.textureSlot")},
		{.name = "material.texture_height", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.height.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
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
