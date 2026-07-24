#include "deferredGeometry.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../context/renderData.hpp"
#include "../context/renderContext.hpp"
#include "../context/visibleObject.hpp"
#include "../context/renderQueue.hpp"
#include "../../ECS/components/bv.hpp"
#include "../../config/configManager.h"

DeferredGeometryPass::DeferredGeometryPass() = default;

DeferredGeometryPass::~DeferredGeometryPass() = default;

void DeferredGeometryPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("gBuffer");
	mObjects = &ctx.queueRegistry->get<VisibleObject>("visibleDeferred");

	const TextureBinding textureBindings[] = {
		{.name = "material.texture_albedo", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.albedo.textureSlot")},
		{.name = "material.texture_normal", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.normal.textureSlot")},
		{
			.name = "material.texture_roughnessMetallic",
			.slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.roughnessMetallic.textureSlot")
		},
		{.name = "material.texture_ao", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.ao.textureSlot")},
		{.name = "material.texture_emissive", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.emissive.textureSlot")},
		{.name = "material.texture_height", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("PBR.height.textureSlot")},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

void DeferredGeometryPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	graph.getResource("gBuffer").bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShader->bind();

	for (const auto& [entityID, materialIdx, textureOffset, textureCount, meshIdx, shader]: *mObjects) {
		RenderCommand::setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *mShader);
		RenderCommand::setupTransform(entityID, ctx, *mShader);

		const uint32_t vao = ctx.renderData->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderData->mesh.indexCounts[meshIdx];

		RenderCommand::drawMesh(vao, vertexCount, indexCount);
	}
}
