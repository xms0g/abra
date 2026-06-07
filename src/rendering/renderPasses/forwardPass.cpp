#include "forwardPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"

ForwardPass::~ForwardPass() = default;

void ForwardPass::configure(RenderContext& ctx, EventBus& eventBus) {
	RenderCommand::bindShadowMaps(ctx);
}

void ForwardPass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();

	if (!ctx.renderQueue->blendObjects.empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommand::forward(ctx, ctx.renderQueue->blendObjects);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommand::forward(ctx, ctx.renderQueue->opaqueObjects);
}
