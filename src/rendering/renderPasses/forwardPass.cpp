#include "forwardPass.h"
#include "glad/glad.h"
#include "../renderCommon.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"

ForwardPass::~ForwardPass() = default;

void ForwardPass::configure(const RenderContext& ctx) {
}

void ForwardPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	RenderCommon::bindShadowMaps(ctx);

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
