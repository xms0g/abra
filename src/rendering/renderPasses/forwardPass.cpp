#include "forwardPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"

ForwardPass::~ForwardPass() = default;

void ForwardPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mOpaqueObjects = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleOpaque");
	mTransparentObjects = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleBlend");

	RenderCommand::bindShadowMaps(ctx);
}

void ForwardPass::execute(const RenderContext& ctx, RenderGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	if (!mOpaqueObjects->empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommand::forward(ctx, *mTransparentObjects);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommand::forward(ctx, *mOpaqueObjects);
}
