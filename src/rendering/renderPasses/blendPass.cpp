#include "blendPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderCommand.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/entity.hpp"

BlendPass::~BlendPass() = default;

void BlendPass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matBatches]: ctx.renderQueue->blendGroups) {
		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();
			shader->setInt("shadowMap", ctx.shadowMap.textureSlot);
			shader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
			shader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);
		}
	}
}

void BlendPass::execute(const RenderContext& ctx) {
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ctx.sceneBuffer->bind();

	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;

	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->blendCommands) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = nullptr;
		}

		RenderCommon::setupTransform(*entity, *lastShader);

		if (lastMaterial != material) {
			RenderCommon::setupMaterial(*entity, *material, *lastShader);
			RenderCommon::bindTextures(material->textures, *lastShader);
			lastMaterial = material;
		}

		RenderCommon::drawMeshes(*mesh);
	}
	if (lastMaterial)
		RenderCommon::unbindTextures(lastMaterial->textures);
	ctx.sceneBuffer->unbind();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
