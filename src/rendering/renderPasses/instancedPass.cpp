#include "instancedPass.h"
#include "glad/glad.h"
#include "../buffers/frameBuffer.h"
#include "../buffers/vertexBuffer.h"
#include "../renderCommand.h"
#include "../shader.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/instanceGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../../math/matrix.h"

struct InstanceData {
	glm::mat4 modelMatrix;
	glm::mat3 normalMatrix;
	float padding[3];
};

InstancedPass::InstancedPass() = default;

InstancedPass::~InstancedPass() = default;

void InstancedPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	if (!ctx.renderQueue->opaqueInstancedGroups.empty()) {
		prepareInstanceBuffer(ctx.renderQueue->opaqueInstancedGroups, ctx.renderQueue->mesh.vaos, mOpaqueVBO);
		uploadInstanceData(ctx.renderQueue->opaqueInstancedGroups, *mOpaqueVBO);
	}

	if (!ctx.renderQueue->blendInstancedGroups.empty()) {
		prepareInstanceBuffer(ctx.renderQueue->blendInstancedGroups, ctx.renderQueue->mesh.vaos, mBlendVBO);
		uploadInstanceData(ctx.renderQueue->blendInstancedGroups, *mBlendVBO);
	}
}

void InstancedPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	RenderCommand::bindShadowMaps(ctx);

	if (!ctx.renderQueue->blendInstancedGroups.empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommand::instanced(ctx, ctx.renderQueue->blendInstancedGroups);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommand::instanced(ctx, ctx.renderQueue->opaqueInstancedGroups);
}

void InstancedPass::prepareInstanceBuffer(
	const std::vector<InstanceGroup>& groups,
	const std::vector<uint32_t>& vaos,
	std::unique_ptr<VertexBuffer>& vbo) {
	vbo = std::make_unique<VertexBuffer>(DYNAMIC);

	size_t totalRequiredSize = 0;
	for (const auto& [entity, transforms, matb] : groups) {
		const size_t instanceCount = transforms.size() / 9;
		totalRequiredSize += instanceCount * sizeof(InstanceData);
	}

	// Allocate the full block of memory once
	vbo->bind();
	vbo->setData(nullptr, static_cast<uint32_t>(totalRequiredSize), 0);

	// Setup attributes now that the buffer is allocated
	uint32_t currentOffset = 0;
	for (const auto& [entity, transforms, matb] : groups) {
		for (const auto meshIdx : matb.meshIndices) {
			const uint32_t vao = vaos[meshIdx];
			Mesh::enableInstanceAttributes(vao, currentOffset);
		}
		const size_t instanceCount = transforms.size() / 9;
		currentOffset += static_cast<uint32_t>(instanceCount * sizeof(InstanceData));
	}

	vbo->unbind();
}

void InstancedPass::uploadInstanceData(const std::vector<InstanceGroup>& groups, const VertexBuffer& vbo) {
	vbo.bind();

	uint32_t currentOffset = 0;
	for (const auto& [entity, transforms, matb]: groups) {
		std::vector<InstanceData> gpuData;
		const size_t instanceCount = transforms.size() / 9;
		gpuData.reserve(instanceCount);

		for (size_t i = 0; i < transforms.size(); i += 9) {
			glm::vec3 pos{transforms[i], transforms[i + 1], transforms[i + 2]};
			glm::vec3 rot{transforms[i + 3], transforms[i + 4], transforms[i + 5]};
			glm::vec3 scale{transforms[i + 6], transforms[i + 7], transforms[i + 8]};

			glm::mat4 model = math::modelMatrix(pos, rot, scale);
			gpuData.emplace_back(model, math::normalMatrix(model));
		}

		const size_t uploadSize = gpuData.size() * sizeof(InstanceData);
		vbo.setData(gpuData.data(), static_cast<uint32_t>(uploadSize), currentOffset);
		currentOffset += static_cast<uint32_t>(uploadSize);
	}

	vbo.unbind();
}
