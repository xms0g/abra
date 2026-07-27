#include "resolve.h"
#include "../frameGraph.h"
#include "../context/renderContext.hpp"

ResolvePass::ResolvePass() = default;

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
	mEncoder = GraphicsEncoder{};
}

void ResolvePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	const auto& sceneBuffer = graph.getResource("sceneBuffer");
	const auto& intermediateBuffer = graph.getResource("intermediateBuffer");
	mEncoder.blitFramebuffer(sceneBuffer, intermediateBuffer, BlitMask::Color);
	//ctx.sceneBuffer = ctx.intermediateBuffer;
}
