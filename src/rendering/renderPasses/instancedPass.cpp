#include "instancedPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../renderContext/renderData.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../material/material.hpp"
#include "../buffers/frameBuffer.h"
#include "../buffers/vertexBuffer.h"
#include "../mesh/mesh.h"
#include "../../math/matrix.h"
#include "../../config/configManager.h"

InstancedPass::InstancedPass() {

}

InstancedPass::~InstancedPass() = default;

void InstancedPass::configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) {
	mOpaqueObjects = &ctx.queueRegistry->get<RenderInstanceGroup>("opaqueInstanced");
	mTransparentObjects = &ctx.queueRegistry->get<RenderInstanceGroup>("blendInstanced");

	if (!mOpaqueObjects->empty()) {
		prepareInstanceBuffer(*mOpaqueObjects, ctx.renderData->mesh.vaos, mOpaqueVBO);
		uploadInstanceData(*mOpaqueObjects, *mOpaqueVBO);
	}

	if (!mTransparentObjects->empty()) {
		prepareInstanceBuffer(*mTransparentObjects, ctx.renderData->mesh.vaos, mBlendVBO);
		uploadInstanceData(*mTransparentObjects, *mBlendVBO);
	}

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	graph.getResource("directional").bindTexture(slot);
	graph.getResource("point").bindTexture(slot + 1);
	graph.getResource("spot").bindTexture(slot + 2);
}

void InstancedPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	if (!mTransparentObjects->empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommand::instanced(ctx, *mTransparentObjects);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommand::instanced(ctx, *mOpaqueObjects);
}

void InstancedPass::prepareInstanceBuffer(
	const RenderQueue<RenderInstanceGroup>& groups,
	const std::vector<uint32_t>& vaos,
	std::unique_ptr<VertexBuffer>& vbo) {
	vbo = std::make_unique<VertexBuffer>(DYNAMIC);

	size_t totalRequiredSize = 0;
	for (const auto& group : groups) {
		const size_t instanceCount = group.transforms.size() / 9;
		totalRequiredSize += instanceCount * sizeof(InstanceData);
	}

	// Allocate the full block of memory once
	vbo->bind();
	vbo->setData(nullptr, static_cast<uint32_t>(totalRequiredSize), 0);

	// Setup attributes now that the buffer is allocated
	uint32_t currentOffset = 0;
	for (const auto& group : groups) {
		for (const auto meshIdx : group.matBatch.meshIndices) {
			const uint32_t vao = vaos[meshIdx];
			Mesh::enableInstanceAttributes(vao, currentOffset);
		}
		const size_t instanceCount = group.transforms.size() / 9;
		currentOffset += static_cast<uint32_t>(instanceCount * sizeof(InstanceData));
	}

	vbo->unbind();
}

void InstancedPass::uploadInstanceData(const RenderQueue<RenderInstanceGroup>& groups, const VertexBuffer& vbo) {
	vbo.bind();

	uint32_t currentOffset = 0;
	for (const auto& group: groups) {
		std::vector<InstanceData> gpuData;
		const size_t instanceCount = group.transforms.size() / 9;
		gpuData.reserve(instanceCount);

		for (size_t i = 0; i < group.transforms.size(); i += 9) {
			glm::vec3 pos{group.transforms[i], group.transforms[i + 1], group.transforms[i + 2]};
			glm::vec3 rot{group.transforms[i + 3], group.transforms[i + 4], group.transforms[i + 5]};
			glm::vec3 scale{group.transforms[i + 6], group.transforms[i + 7], group.transforms[i + 8]};

			glm::mat4 model = math::modelMatrix(pos, rot, scale);
			gpuData.emplace_back(model, math::normalMatrix(model));
		}

		const size_t uploadSize = gpuData.size() * sizeof(InstanceData);
		vbo.setData(gpuData.data(), static_cast<uint32_t>(uploadSize), currentOffset);
		currentOffset += static_cast<uint32_t>(uploadSize);
	}

	vbo.unbind();
}
