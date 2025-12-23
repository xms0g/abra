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

BlendInstancedPass::~BlendInstancedPass() = default;

void BlendInstancedPass::configure(const RenderContext& ctx) {
	InstanceBufferBuilder::prepareInstanceBuffer(ctx.renderQueue->blendInstancedGroups, mVBO);
	InstanceBufferBuilder::uploadInstanceData(ctx.renderQueue->blendInstancedGroups, mVBO);
}

void BlendInstancedPass::execute(const RenderContext& ctx) {
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ctx.sceneBuffer->bind();
	for (const auto& [entity, transforms, matb]: ctx.renderQueue->blendInstancedGroups) {
		const size_t count = transforms->size() / 9;

		for (const auto& [material, shader, meshes] = matb; const auto& mesh: *meshes) {
			RenderCommon::setupMaterial(entity, *material, *shader);
			RenderCommon::bindTextures(material->textures, *shader);

			mesh.bind();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<int32_t>(mesh.indices().size()),
			                        GL_UNSIGNED_INT, nullptr, static_cast<int32_t>(count));

			RenderCommon::unbindTextures(material->textures);
		}
	}
	ctx.sceneBuffer->unbind();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}