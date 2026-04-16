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
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/transform.hpp"

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
			setupMaterial(*entity, *material, *lastShader);
			bindTextures(material->textures, *lastShader);
			lastMaterial = material;
		}

		drawMesh(*mesh);
	}
}

void RenderCommon::instanced(const std::vector<InstanceGroup>& objects) {
	for (const auto& [entity, transforms, matBatch]: objects) {
		const size_t count = transforms->size() / 9;

		const auto& [material, shader, meshes] = matBatch;
		shader->activate();

		for (const auto& mesh: *meshes) {
			setupMaterial(entity, *material, *shader);
			bindTextures(material->textures, *shader);

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
	const glm::mat4 model = math::modelMatrix(
		entity.transform->position,
		entity.transform->rotation,
		entity.transform->scale);
	const glm::mat3 normal = math::normalMatrix(model);

	shader.setMat4("model", model);
	shader.setMat3("normalMatrix", normal);
}

void RenderCommon::setupMaterial(const EntityCore& entity, const Material& material, const Shader& shader) {
	shader.setFloat("material.heightScale", entity.material->heightScale);
	shader.setFloat("material.alphaCutoff", material.alphaCutoff);

	if (material.textures.empty()) {
		shader.setVec3("material.color", material.color);
	}

	if (material.flags & TWOSIDED) {
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
	uint32_t flags{0};
	uint32_t roughMetalID{0};

	for (size_t i = 0; i < textures.size(); ++i) {
		switch (textures[i].type) {
			case HEIGHT:
				flags |= HAS_HEIGHT_MAP;
				break;
			case EMISSION:
				flags |= HAS_EMISSIVE_MAP;
				break;
			case ROUGHNESS_METALLIC:
				roughMetalID = textures[i].id;
				break;
			case AO: {
				if (roughMetalID == textures[i].id) {
					flags |= HAS_ORM;
					continue;
				}

				flags |= HAS_AO_MAP;
				break;
			}
			default: break;
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	shader.setUint("material.flags", flags);
}

void RenderCommon::bindShadowMaps(const RenderContext& ctx) {
	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot);
	glBindTexture(GL_TEXTURE_2D, ctx.shadow.textures->at(0));

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, ctx.shadow.textures->at(1));

	glActiveTexture(GL_TEXTURE0 + ctx.shadow.textureSlot + 2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ctx.shadow.textures->at(2));
}
