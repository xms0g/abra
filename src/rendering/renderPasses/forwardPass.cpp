#include "forwardPass.h"
#include "glad/glad.h"
#include "../renderCommand.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"
#include "../renderContext/renderQueue.hpp"
#include "../renderContext/renderableObject.hpp"
#include "../../config/configManager.h"

ForwardPass::ForwardPass(const RenderContext& ctx, const RenderGraph& graph) {
	mInputs = {"sceneBuffer", "visibleOpaque", "visibleBlend"};
	mOutputs = {"sceneBuffer"};
	mOpaqueObjects = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleOpaque");
	mTransparentObjects = &ctx.renderQueue->get<std::vector<RenderableObject> >("visibleBlend");

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	graph.getResource("directional").bindTexture(slot);
	graph.getResource("point").bindTexture(slot + 1);
	graph.getResource("spot").bindTexture(slot + 2);
}

ForwardPass::~ForwardPass() = default;

void ForwardPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
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
