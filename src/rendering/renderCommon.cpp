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
#include "../math/matrix.h"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"

void RenderCommon::forward(const RenderContext& ctx, const std::vector<RenderableObject>& objects) {
	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;

	for (const auto& [entityID, material, shader, mesh]: objects) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = nullptr;
		}

		auto& [position, rotation, scale] = ctx.renderQueue->entityTransforms.at(entityID);

		setupTransform(position, rotation, scale, *lastShader);

		if (lastMaterial != material) {
			if (material->textures.empty()) {
				shader->setVec3("material.color", material->color);
			}

			const float heightScale = ctx.renderQueue->entityHeightScales.at(entityID);

			setupMaterial(*material, *lastShader, heightScale);
			bindTextures(*material, *lastShader);
			lastMaterial = material;
		}

		drawMesh(*mesh);
	}
}

void RenderCommon::instanced(const RenderContext& ctx, const std::vector<InstanceGroup>& objects) {
	for (const auto& [entityID, transforms, matBatch]: objects) {
		const size_t count = transforms->size() / 9;

		const auto& [material, shader, meshes] = matBatch;
		shader->activate();

		const float heightScale = ctx.renderQueue->entityHeightScales.at(entityID);
		setupMaterial(*material, *shader, heightScale);
		bindTextures(*material, *shader);

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
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const Shader& shader) {
	const glm::mat4 model = math::modelMatrix(position, rotation, scale);
	const glm::mat3 normal = math::normalMatrix(model);

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommon::setupMaterial(const Material& material, const Shader& shader, const float heightScale) {
	static bool isCullingEnabled{true};

	if (material.flags & HAS_HEIGHT_MAP) {
		shader.setFloat("material.heightScale", heightScale);
	}

	if (material.alphaCutoff != 0.0f) {
		shader.setFloat("material.alphaCutoff", material.alphaCutoff);
	}

	if (material.flags & TWOSIDED && isCullingEnabled) {
		glDisable(GL_CULL_FACE);
		isCullingEnabled = false;
	} else if (!(material.flags & TWOSIDED) && !isCullingEnabled) {
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


void RenderCommon::bindTextures(const Material& material, const Shader& shader) {
	static uint32_t matFlagCache{0};

	for (size_t i = 0; i < material.textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, material.textures[i].id);
	}

	if (matFlagCache != material.flags) {
		matFlagCache = material.flags;
		shader.setUint("material.flags", material.flags);
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