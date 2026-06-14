#include "renderCommand.h"
#include "glad/glad.h"
#include "shader.h"
#include "material/material.hpp"
#include "texture/texture.h"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderContext/renderQueue.hpp"
#include "renderContext/renderGroup.hpp"
#include "../ECS/components/mesh.hpp"

void RenderCommand::forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects) {
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

void RenderCommand::instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects) {
	for (const auto& obj: objects) {
		const size_t count = obj.transforms.size() / 9;

		const auto& [materialIdx, textureOffset, textureCount, shader, meshes] = obj.matBatch;
		shader->activate();

		setupMaterial(obj.entityID, materialIdx, textureOffset, textureCount, ctx, *shader);

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

void RenderCommand::setupTransform(const size_t entityID, const RenderContext& ctx, const Shader& shader) {
	const auto& model = ctx.renderQueue->entity.models[entityID];
	const auto& normal = ctx.renderQueue->entity.normals[entityID];

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommand::setupMaterial(
	const size_t entityID,
	const uint32_t materialIdx,
	const uint32_t textureOffset,
	const size_t textureCount,
	const RenderContext& ctx,
	const Shader& shader) {
	static bool isCullingEnabled{true};

	if (ctx.materialCache.lastMaterialIdx == materialIdx) {
		return;
	}

	ctx.materialCache.lastMaterialIdx = materialIdx;

	const uint32_t flags = ctx.renderQueue->material.flags[materialIdx];

	if (ctx.materialCache.lastMatFlags != flags || ctx.materialCache.lastShader != &shader) {
		ctx.materialCache.lastMatFlags = flags;
		ctx.materialCache.lastShader = &shader;
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

void RenderCommand::setTextureUnits(const std::vector<TextureBinding>& textures, const Shader& shader) {
	shader.activate();

	for (const auto& [name, slot]: textures) {
		shader.setInt(name, slot);
	}
}

void RenderCommand::drawMesh(const uint32_t vao, const size_t vertexCount, const size_t indexCount) {
	glBindVertexArray(vao);

	if (indexCount > 0) [[likely]] {
		glDrawElements(GL_TRIANGLES, static_cast<int32_t>(indexCount), GL_UNSIGNED_INT, nullptr);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(vertexCount));
	}
}

void RenderCommand::drawQuad(const uint32_t vao) {
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void RenderCommand::drawQuad(const uint32_t vao, const std::span<const uint32_t> textures) {
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);

	for (uint32_t i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void RenderCommand::bindShadowMaps(const RenderContext& ctx) {
	struct ShadowBinding {
		int32_t slot;
		uint32_t target;
		uint32_t mID;
	};

	const ShadowBinding shadowBindings[] = {
		{GL_TEXTURE0 + ctx.shadow.textureSlot, GL_TEXTURE_2D, ctx.renderQueue->shadowMaps[0]},
		{GL_TEXTURE0 + ctx.shadow.textureSlot + 1, GL_TEXTURE_CUBE_MAP_ARRAY, ctx.renderQueue->shadowMaps[1]},
		{GL_TEXTURE0 + ctx.shadow.textureSlot + 2, GL_TEXTURE_2D_ARRAY, ctx.renderQueue->shadowMaps[2]},
	};

	for (const auto& [slot, target, mID]: shadowBindings) {
		glActiveTexture(slot);
		glBindTexture(target, mID);
	}
}
