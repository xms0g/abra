#include "renderCommon.h"
#include "glad/glad.h"
#include "shader.h"
#include "material/material.hpp"
#include "texture/texture.h"
#include "mesh/mesh.h"
#include "renderContext/renderContext.hpp"
#include "renderContext/entityData.hpp"
#include "../math/matrix.h"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/transform.hpp"

void RenderCommon::setupTransform(const EntityData& entity, const Shader& shader) {
	const glm::mat4 model = math::modelMatrix(
		entity.transform->position,
		entity.transform->rotation,
		entity.transform->scale);
	const glm::mat3 normal = math::normalMatrix(model);

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommon::setupMaterial(const EntityData& entity, const Material& material, const Shader& shader) {
	shader.setFloat("material.heightScale", entity.material->heightScale);
	shader.setFloat("material.alphaCutout", material.alphaCutout);

	if (material.textures.empty()) {
		shader.setVec3("material.color", material.color);
	}

	if (material.flag & TWOSIDED) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
	}
}

void RenderCommon::drawMesh(const Mesh& mesh) {
	mesh.bind();
	if (!mesh.indices().empty()) {
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


void RenderCommon::bindTextures(const std::vector<Texture>& textures, const Shader& shader) {
	bool hasHeightMap{false};
	bool hasEmissiveMap{false};
	bool hasAOMap{false};
	bool hasORM{false};

	std::string_view roughMetal;
	for (size_t i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding

		if (textures[i].type == HEIGHT) {
			hasHeightMap = true;
		}

		if (textures[i].type == EMISSION) {
			hasEmissiveMap = true;
		}

		if (textures[i].type == ROUGHNESS_METALLIC) {
			roughMetal = textures[i].path;
		}

		if (textures[i].type == AO) {
			if (roughMetal == textures[i].path) {
				hasORM = true;
				continue;
			}
			
			hasAOMap = true;
		}

		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	shader.setBool("material.hasHeightMap", hasHeightMap);
	shader.setBool("material.hasEmissiveMap", hasEmissiveMap);
	shader.setBool("material.hasAOMap", hasAOMap);
	shader.setBool("material.hasORM", hasORM);
}

void RenderCommon::unbindTextures(const std::vector<Texture>& textures) {
	for (size_t i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void RenderCommon::bindShadowMaps(const RenderContext& ctx) {
	static uint32_t dirShadowMapSlot = ctx.shadow.textures->at(0);
	static uint32_t omniShadowMapSlot = ctx.shadow.textures->at(1);
	static uint32_t persShadowMapSlot = ctx.shadow.textures->at(2);

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot);
	glBindTexture(GL_TEXTURE_2D, dirShadowMapSlot);

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, omniShadowMapSlot);

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, persShadowMapSlot);
}
