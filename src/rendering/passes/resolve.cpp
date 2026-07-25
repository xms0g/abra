#include "resolve.h"
#include "../graph.h"
#include "../context/renderContext.hpp"

ResolvePass::ResolvePass() = default;

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mEncoder = GraphicsEncoder{graph};
}

void ResolvePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.blitFramebuffer("sceneBuffer", "intermediateBuffer", BlitMask::Color);
	//ctx.sceneBuffer = ctx.intermediateBuffer;
}
