#include "blendInstancedPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/instanceGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../material/material.hpp"
#include "../mesh/mesh.h"
#include "../../math/matrix.hpp"

BlendInstancedPass::~BlendInstancedPass() = default;

void BlendInstancedPass::configure(const RenderContext& ctx) {
	prepareInstanceBuffer(ctx);
	prepareInstanceData(ctx);
}

void BlendInstancedPass::execute(const RenderContext& ctx) {
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ctx.sceneBuffer->bind();
	for (const auto& [entity, transforms, matBatches]: ctx.renderQueue->blendInstancedGroups) {
		const size_t count = transforms->size() / 9;

		for (const auto& [material, shader, meshes]: matBatches) {
			RenderCommon::setupMaterial(*entity, *shader);
			RenderCommon::bindTextures(material->textures, *shader);

			for (const auto& mesh: *meshes) {
				glBindVertexArray(mesh.VAO());
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<int32_t>(mesh.indices().size()),
										GL_UNSIGNED_INT, nullptr, static_cast<int32_t>(count));
			}

			RenderCommon::unbindTextures(material->textures);
		}
	}
	ctx.sceneBuffer->unbind();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void BlendInstancedPass::prepareInstanceBuffer(const RenderContext& ctx) {
	glGenBuffers(1, &vbo.buffer);

	size_t requiredGPUBufferSize = 0;
	for (const auto& [entity, transforms, materials]: ctx.renderQueue->blendInstancedGroups) {
		for (const auto& material: materials) {
			for (const auto& mesh: *material.meshes) {
				mesh.enableInstanceAttributes(vbo.buffer, vbo.offset);
			}
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

void BlendInstancedPass::prepareInstanceData(const RenderContext& ctx) const {
	for (const auto& [entity, transforms, materials]: ctx.renderQueue->blendInstancedGroups) {
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
