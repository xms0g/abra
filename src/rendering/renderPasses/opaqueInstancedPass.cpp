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

OpaqueInstancedPass::~OpaqueInstancedPass() = default;

void OpaqueInstancedPass::configure(const RenderContext& ctx) {
	InstanceBufferBuilder::prepareInstanceBuffer(ctx.renderQueue->opaqueInstancedGroups, mVBO);
	InstanceBufferBuilder::uploadInstanceData(ctx.renderQueue->opaqueInstancedGroups, mVBO);

	for (const auto& [entity, transforms, matb]: ctx.renderQueue->opaqueInstancedGroups) {
		const auto& [material, shader, meshes] = matb;
		shader->activate();
		shader->setInt("shadowMap", ctx.shadow.textureSlot);
		shader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
		shader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);
	}
}

void OpaqueInstancedPass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(ctx);

	ctx.sceneBuffer->bind();
	for (const auto& [entity, transforms, matb]: ctx.renderQueue->opaqueInstancedGroups) {
		const size_t count = transforms->size() / 9;

		const auto& [material, shader, meshes] = matb;
		for (const auto& mesh: *meshes) {
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
