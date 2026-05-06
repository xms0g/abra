#include "renderCommon.h"
#include "glad/glad.h"
#include "shader.h"
#include "material/material.hpp"
#include "texture/texture.h"
#include "mesh/mesh.h"
#include "mesh/vertex.hpp"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderContext/renderQueue.hpp"
#include "renderContext/instanceGroup.hpp"
#include "../ECS/components/mesh.hpp"

void RenderCommon::forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects) {
	uint32_t lastMaterial{0};
	const Shader* lastShader{nullptr};

	for (const auto& [entityID, model, normal, materialIdx, shader, mesh]: objects) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = 0;
		}

		if (lastMaterial != materialIdx) {
			lastMaterial = materialIdx;

			const float heightScale = ctx.renderQueue->entityHeightScales.at(entityID);
			const float alphaCutoff = ctx.renderQueue->matAlphaCutoffs.at(materialIdx);
			const uint32_t flags = ctx.renderQueue->matFlags.at(materialIdx);
			const std::vector<uint32_t>& textures = ctx.renderQueue->matTextures.at(materialIdx);

			// if (textures.empty()) {
			// 	shader->setVec3("material.color", materialIdx->color);
			// }

			setupMaterial(flags, alphaCutoff, heightScale, *lastShader);
			bindTextures(flags, textures, *lastShader);

		}

		setupTransform(entityID, model, normal, *lastShader);
		drawMesh(*mesh);
	}
}

void RenderCommon::instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects) {
	for (const auto& [entityID, transforms, matBatch]: objects) {
		const size_t count = transforms->size() / 9;

		const auto& [materialIdx, shader, meshes] = matBatch;
		shader->activate();

		const float heightScale = ctx.renderQueue->entityHeightScales.at(entityID);
		const float alphaCutoff = ctx.renderQueue->matAlphaCutoffs.at(materialIdx);
		const uint32_t flags = ctx.renderQueue->matFlags.at(materialIdx);
		const std::vector<uint32_t>& textures = ctx.renderQueue->matTextures.at(materialIdx);

		setupMaterial(flags, alphaCutoff, heightScale, *shader);
		bindTextures(flags, textures, *shader);

		for (const auto& mesh: *meshes) {
			mesh.bind();
			glDrawElementsInstanced(
				GL_TRIANGLES,
				static_cast<int32_t>(mesh.indices().size()),
				GL_UNSIGNED_INT,
				nullptr,
				static_cast<int32_t>(count));
		}
	}
}

void RenderCommon::setupTransform(
	const size_t entityID,
	const glm::mat4& model,
	const glm::mat3& normal,
	const Shader& shader) {
	static size_t lastEntityID{0};

	if (lastEntityID != entityID) {
		lastEntityID = entityID;
		shader.setMat4("model", model);
		shader.setMat3("normalMatrix", normal);
	}
}

void RenderCommon::setupMaterial(const uint32_t flags, const float alphaCutoff, const float heightScale, const Shader& shader) {
	static bool isCullingEnabled{true};

	if (flags & HAS_HEIGHT_MAP) {
		shader.setFloat("material.heightScale", heightScale);
	}

	if (alphaCutoff != 0.0f) {
		shader.setFloat("material.alphaCutoff", alphaCutoff);
	}

	if (flags & TWOSIDED && isCullingEnabled) {
		glDisable(GL_CULL_FACE);
		isCullingEnabled = false;
	} else if (!(flags & TWOSIDED) && !isCullingEnabled) {
		glEnable(GL_CULL_FACE);
		isCullingEnabled = true;
	}
}

void RenderCommon::drawMesh(const Mesh& mesh) {
	mesh.bind();
	if (!mesh.indices().empty()) [[likely]] {
		glDrawElements(GL_TRIANGLES, static_cast<int32_t>(mesh.indices().size()), GL_UNSIGNED_INT, nullptr);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, static_cast<int32_t>(mesh.vertices().size()));
	}
}

void RenderCommon::drawQuad(const uint32_t sceneTexture, const uint32_t VAO) {
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}


void RenderCommon::bindTextures(const uint32_t flags, const std::vector<uint32_t>& textures, const Shader& shader) {
	static uint32_t matFlagCache{0};

	for (size_t i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}

	if (matFlagCache != flags) {
		matFlagCache = flags;
		shader.setUint("material.flags", flags);
	}
}

void RenderCommon::bindShadowMaps(const RenderContext& ctx) {
	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot);
	glBindTexture(GL_TEXTURE_2D, ctx.shadow.textures->at(0));

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, ctx.shadow.textures->at(1));

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ctx.shadow.textures->at(2));
}
