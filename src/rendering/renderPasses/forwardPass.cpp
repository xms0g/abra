#include "forwardPass.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderGroup.hpp"

ForwardPass::~ForwardPass() = default;

void ForwardPass::configure(const RenderContext& ctx) {
	for (const auto& [entity, matb]: ctx.renderQueue->opaqueGroups) {
		const auto& [material, shader, meshes] = matb;
		shader->activate();
		shader->setInt("shadowMap", ctx.shadow.textureSlot);
		shader->setInt("shadowCubemap", ctx.shadow.textureSlot + 1);
		shader->setInt("persShadowMap", ctx.shadow.textureSlot + 2);
	}
}

void ForwardPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();
	if (!ctx.renderQueue->blendObjects.empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommon::forward(ctx.renderQueue->blendObjects);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommon::forward(ctx.renderQueue->opaqueObjects);
	ctx.sceneBuffer->unbind();
}
