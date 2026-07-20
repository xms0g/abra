#include "resolvePass.h"
#include "glad/glad.h"
#include "../renderGraph.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"

ResolvePass::ResolvePass() {
	mReads = {"sceneBuffer"};
	mWrites = {"intermediateBuffer"};
}

ResolvePass::~ResolvePass() = default;

void ResolvePass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	const auto& sceneBuffer = graph.getResource("sceneBuffer");
	const auto& intermediateBuffer = graph.getResource("intermediateBuffer");

	sceneBuffer.bindForRead();
	intermediateBuffer.bindForDraw();
	glBlitFramebuffer(0, 0, sceneBuffer.width(), sceneBuffer.height(),
					  0, 0, intermediateBuffer.width(), intermediateBuffer.height(),
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//ctx.sceneBuffer = ctx.intermediateBuffer;
}
