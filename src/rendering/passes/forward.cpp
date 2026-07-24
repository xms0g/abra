#include "forward.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/visibleObject.hpp"
#include "../../config/configManager.h"

ForwardPass::ForwardPass() = default;

ForwardPass::~ForwardPass() = default;

void ForwardPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mOpaqueObjects = &ctx.queueRegistry->get<VisibleObject>("visibleOpaque");
	mTransparentObjects = &ctx.queueRegistry->get<VisibleObject>("visibleBlend");

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	graph.getResource("directional").bindTexture(slot);
	graph.getResource("point").bindTexture(slot + 1);
	graph.getResource("spot").bindTexture(slot + 2);
}

void ForwardPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	graph.getResource("sceneBuffer").bind();

	if (!mTransparentObjects->empty()) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderCommand::forward(ctx, *mTransparentObjects);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	RenderCommand::forward(ctx, *mOpaqueObjects);
}
