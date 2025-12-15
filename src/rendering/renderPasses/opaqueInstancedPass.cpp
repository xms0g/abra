#include "opaqueInstancedPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/instanceGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../../math/matrix.hpp"

OpaqueInstancedPass::~OpaqueInstancedPass() = default;

void OpaqueInstancedPass::configure(const RenderContext& ctx) {
	prepareInstanceBuffer(ctx);
	prepareInstanceData(ctx);

	for (const auto& [entity, transforms, matBatch]: ctx.renderQueue->opaqueInstancedGroups) {
		const auto& [material, shader, meshes] = matBatch;
		shader->activate();
		shader->setInt("shadowMap", ctx.shadowMap.textureSlot);
		shader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
		shader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);
	}
}

void OpaqueInstancedPass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(*ctx.shadowMap.textures);

	ctx.sceneBuffer->bind();
	for (const auto& [entity, transforms, matBatch]: ctx.renderQueue->opaqueInstancedGroups) {
		const size_t count = transforms->size() / 9;

		for (const auto& [material, shader, meshes] = matBatch; const auto& mesh: *meshes) {
			shader->activate();

			RenderCommon::setupMaterial(entity, *material, *shader);
			RenderCommon::bindTextures(material->textures, *shader);

			mesh.bind();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<int32_t>(mesh.indices().size()),
			                        GL_UNSIGNED_INT, nullptr, static_cast<int32_t>(count));

			RenderCommon::unbindTextures(material->textures);
		}
	}
	ctx.sceneBuffer->unbind();
}

void OpaqueInstancedPass::prepareInstanceBuffer(const RenderContext& ctx) {
	glGenBuffers(1, &vbo.buffer);

	size_t requiredGPUBufferSize = 0;
	for (const auto& [entity, transforms, matBatch]: ctx.renderQueue->opaqueInstancedGroups) {
		for (const auto& mesh: *matBatch.meshes) {
			mesh.enableInstanceAttributes(vbo.buffer, vbo.offset);
		}

		const size_t count = transforms->size() / 9;
		const size_t instanceSize = count * sizeof(InstanceData);
		requiredGPUBufferSize += instanceSize;
		vbo.offset += static_cast<int>(instanceSize);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo.buffer);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(requiredGPUBufferSize), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vbo.offset = 0;
}

void OpaqueInstancedPass::prepareInstanceData(const RenderContext& ctx) const {
	for (const auto& [entity, transforms, materials]: ctx.renderQueue->opaqueInstancedGroups) {
		std::vector<InstanceData> gpuData;
		gpuData.reserve(transforms->size() / 9);

		auto transform = *transforms;
		for (int i = 0; i < transform.size(); i += 9) {
			glm::vec3 pos{transform[i], transform[i + 1], transform[i + 2]};
			glm::vec3 rot{transform[i + 3], transform[i + 4], transform[i + 5]};
			glm::vec3 scale{transform[i + 6], transform[i + 7], transform[i + 8]};

			const glm::mat4 model = math::computeModelMatrix(pos, rot, scale);
			const glm::mat3 normal = math::computeNormalMatrix(model);
			gpuData.emplace_back(model, normal);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo.buffer);
		glBufferSubData(GL_ARRAY_BUFFER, vbo.offset, static_cast<long>(gpuData.size() * sizeof(InstanceData)),
		                gpuData.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
