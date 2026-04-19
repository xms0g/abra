#include "renderCommon.h"
#include "glad/glad.h"
#include "shader.h"
#include "material/material.hpp"
#include "texture/texture.h"
#include "mesh/mesh.h"
#include "mesh/vertex.hpp"
#include "renderContext/renderContext.hpp"
#include "renderContext/renderableObject.hpp"
#include "renderContext/instanceGroup.hpp"
#include "renderContext/entityData.hpp"
#include "../math/matrix.h"
#include "../ECS/components/mesh.hpp"

void RenderCommon::forward(const std::vector<RenderableObject>& objects) {
	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;

	for (const auto& [entity, material, shader, mesh]: objects) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = nullptr;
		}

		setupTransform(*entity, *lastShader);

		if (lastMaterial != material) {
			if (material->textures.empty()) {
				shader->setVec3("material.color", material->color);
			}

			setupMaterial(*entity, *material, *lastShader);
			bindTextures(*material, *lastShader);
			lastMaterial = material;
		}

		drawMesh(mesh->vao(), mesh->vertices().size(), mesh->indices().size());
	}
}

void RenderCommon::instanced(const std::vector<InstanceGroup>& objects) {
	for (const auto& [entity, transforms, matBatch]: objects) {
		const size_t count = transforms->size() / 9;

		const auto& [material, shader, meshes] = matBatch;
		shader->activate();

		for (const auto& mesh: *meshes) {
			setupMaterial(entity, *material, *shader);
			bindTextures(*material, *shader);

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

void RenderCommon::setupTransform(const EntityCore& entity, const Shader& shader) {
	static const EntityCore* lastEntity{nullptr};

	if (lastEntity == &entity) {
		return;
	}

	lastEntity = &entity;

	const glm::mat4 model = math::modelMatrix(entity.position, entity.rotation, entity.scale);
	const glm::mat3 normal = math::normalMatrix(model);

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommon::setupMaterial(const EntityCore& entity, const Material& material, const Shader& shader) {
	static bool isCullingEnabled{true};

	if (material.flags & HAS_HEIGHT_MAP) {
		shader.setFloat("material.heightScale", entity.heightScale);
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

void RenderCommon::drawMesh(const uint32_t vao, const uint32_t vertexCount, const uint32_t indexCount) {
	glBindVertexArray(vao);

	if (indexCount) {
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