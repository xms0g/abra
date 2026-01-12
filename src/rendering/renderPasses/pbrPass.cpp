#include "pbrPass.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../renderContext/renderGroup.hpp"
#include "../../ECS/entity.hpp"

PBRPass::~PBRPass() = default;

void PBRPass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matb]: ctx.renderQueue->pbrGroups) {
		const auto& [material, shader, meshes] = matb;
		shader->activate();
		// shader->setInt("shadowMap", ctx.shadow.textureSlot);
		// shader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
		// shader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);
	}
}

void PBRPass::execute(const RenderContext& ctx) {
	RenderCommon::bindShadowMaps(ctx);
	ctx.sceneBuffer->bind();

	const Material* lastMaterial = nullptr;
	const Shader* lastShader = nullptr;
	for (const auto& [entity, material, shader, mesh]: ctx.renderQueue->pbrObjects) {
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

		RenderCommon::drawMesh(*mesh);
	}
	if (lastMaterial)
		RenderCommon::unbindTextures(lastMaterial->textures);
	ctx.sceneBuffer->unbind();
}
