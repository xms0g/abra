#include "forwardOpaquePass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderCommand.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/entity.hpp"

ForwardOpaquePass::~ForwardOpaquePass() = default;

void ForwardOpaquePass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matBatches]: ctx.renderQueue->forwardOpaqueGroups) {
		for (const auto& [material, shader, meshes]: matBatches) {
			shader->activate();
			shader->setInt("shadowMap", ctx.shadowMap.textureSlot);
			shader->setInt("shadowCubemap", ctx.shadowMap.textureSlot + 1);
			shader->setInt("persShadowMap", ctx.shadowMap.textureSlot + 2);
		}
	}
}

void ForwardOpaquePass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(*ctx.shadowMap.textures);
	ctx.sceneBuffer->bind();

	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->forwardCommands) {
		if (lastShader != shader) {
			lastShader = shader;
			lastShader->activate();
			lastMaterial = nullptr;
		}

		RenderCommon::setupTransform(*entity, *lastShader);

		if (lastMaterial != material) {
			lastMaterial = material;
			RenderCommon::setupMaterial(*entity, *lastMaterial, *lastShader);
			RenderCommon::bindTextures(lastMaterial->textures, *lastShader);
		}

		RenderCommon::drawMeshes(*mesh);
	}
	if (lastMaterial)
		RenderCommon::unbindTextures(lastMaterial->textures);
	ctx.sceneBuffer->unbind();
}


