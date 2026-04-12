#include "instancedPass.h"
#include "glad/glad.h"
#include "../buffers/frameBuffer.h"
#include "../renderCommon.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/instanceGroup.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../material/material.hpp"
#include "../mesh/mesh.h"

InstancedPass::~InstancedPass() = default;

void InstancedPass::configure(const RenderContext& ctx) {
	if (!ctx.renderQueue->opaqueInstancedGroups.empty()) {
		prepareInstanceBuffer(ctx.renderQueue->opaqueInstancedGroups, mOpaqueVBO);
		uploadInstanceData(ctx.renderQueue->opaqueInstancedGroups, *mOpaqueVBO);
	}

	if (!ctx.renderQueue->blendInstancedGroups.empty()) {
		prepareInstanceBuffer(ctx.renderQueue->blendInstancedGroups, mBlendVBO);
		uploadInstanceData(ctx.renderQueue->blendInstancedGroups, *mBlendVBO);
	}
}

void InstancedPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	RenderCommon::bindShadowMaps(ctx);

	if (!ctx.renderQueue->blendInstancedGroups.empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommon::instanced(ctx.renderQueue->blendInstancedGroups);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommon::instanced(ctx.renderQueue->opaqueInstancedGroups);
	ctx.sceneBuffer->unbind();
}
