#include "renderCommon.h"
#include "glad/glad.h"
#include "shader.h"
#include "material/material.hpp"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderContext/renderQueue.hpp"
#include "renderContext/instanceGroup.hpp"
#include "../ECS/components/mesh.hpp"

void RenderCommon::forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects) {
	const Shader* lastShader{nullptr};

	for (const auto& [entityID, materialIdx, textureOffset, textureCount, meshIdx, shader]: objects) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
		}

		setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *lastShader);
		setupTransform(entityID, ctx, *lastShader);

		const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
		const size_t vertexCount = ctx.renderQueue->mesh.vertexCounts[meshIdx];
		const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

		drawMesh(vao, vertexCount, indexCount);
	}
}

void RenderCommon::instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects) {
	for (const auto& [entityID, transforms, matBatch]: objects) {
		const size_t count = transforms->size() / 9;

		const auto& [materialIdx, textureOffset, textureCount, shader, meshes] = matBatch;
		shader->activate();

		setupMaterial(entityID, materialIdx, textureOffset, textureCount, ctx, *shader);

		for (const auto& meshIdx: meshes) {
			const uint32_t vao = ctx.renderQueue->mesh.vaos[meshIdx];
			const size_t indexCount = ctx.renderQueue->mesh.indexCounts[meshIdx];

			glBindVertexArray(vao);
			glDrawElementsInstanced(
				GL_TRIANGLES,
				static_cast<int32_t>(indexCount),
				GL_UNSIGNED_INT,
				nullptr,
				static_cast<int32_t>(count));
		}
	}
}

void RenderCommon::setupTransform(const size_t entityID, const RenderContext& ctx, const Shader& shader) {
	const auto& model = ctx.renderQueue->entity.models[entityID];
	const auto& normal = ctx.renderQueue->entity.normals[entityID];

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommon::setupMaterial(
	const size_t entityID,
	const uint32_t materialIdx,
	const uint32_t textureOffset,
	const size_t textureCount,
	const RenderContext& ctx,
	const Shader& shader) {
	static bool isCullingEnabled{true};
	static uint32_t lastMaterialIdx{0};
	static uint32_t lastMatFlags{0};
	static const Shader* lastShader{nullptr};

	if (lastMaterialIdx == materialIdx) {
		return;
	}

	lastMaterialIdx = materialIdx;

	const uint32_t flags = ctx.renderQueue->material.flags[materialIdx];

	if (lastMatFlags != flags || lastShader != &shader) {
		lastMatFlags = flags;
		lastShader = &shader;
		shader.setUint("material.flags", flags);
	}

	if (flags & HAS_HEIGHT_MAP) {
		const float heightScale = ctx.renderQueue->entity.heightScales[entityID];
		shader.setFloat("material.heightScale", heightScale);
	}

	if (flags & ALPHACUTOFF) {
		const float alphaCutoff = ctx.renderQueue->material.alphaCutoffs[materialIdx];
		shader.setFloat("material.alphaCutoff", alphaCutoff);
	}

	if (flags & HAS_SOLID_COLOR) [[unlikely]] {
		const glm::vec3& color = ctx.renderQueue->material.colors[materialIdx];
		shader.setVec3("material.color", color);
	} else {
		int slot{0};
		for (uint32_t i = textureOffset; i < textureOffset + textureCount; ++i) {
			glActiveTexture(GL_TEXTURE0 + slot++);
			glBindTexture(GL_TEXTURE_2D, ctx.renderQueue->material.textures[i]);
		}
	}

	if (flags & TWOSIDED && isCullingEnabled) [[unlikely]] {
		glDisable(GL_CULL_FACE);
		isCullingEnabled = false;
	} else if (!(flags & TWOSIDED) && !isCullingEnabled) {
		glEnable(GL_CULL_FACE);
		isCullingEnabled = true;
	}
}

void RenderCommon::drawMesh(const uint32_t vao, const size_t vertexCount, const size_t indexCount) {
	glBindVertexArray(vao);

	if (indexCount > 0) [[likely]] {
		glDrawElements(GL_TRIANGLES, static_cast<int32_t>(indexCount), GL_UNSIGNED_INT, nullptr);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(vertexCount));
	}
}

void RenderCommon::drawQuad(const uint32_t sceneTexture, const uint32_t vao) {
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void RenderCommon::bindShadowMaps(const RenderContext& ctx) {
	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot);
	glBindTexture(GL_TEXTURE_2D, ctx.renderQueue->shadowMaps[0]);

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, ctx.renderQueue->shadowMaps[1]);

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ctx.renderQueue->shadowMaps[2]);
}
