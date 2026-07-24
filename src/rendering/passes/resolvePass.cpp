#include "resolvePass.h"
#include "glad/glad.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../context/renderContext.hpp"

ResolvePass::ResolvePass() = default;

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
}

void ResolvePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& sceneBuffer = graph.getResource("sceneBuffer");
	const auto& intermediateBuffer = graph.getResource("intermediateBuffer");

	sceneBuffer.bindForRead();
	intermediateBuffer.bindForDraw();
	glBlitFramebuffer(0, 0, sceneBuffer.width(), sceneBuffer.height(),
					  0, 0, intermediateBuffer.width(), intermediateBuffer.height(),
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//ctx.sceneBuffer = ctx.intermediateBuffer;
}
