#include "resolve.h"
#include "../frameGraph.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"

ResolvePass::ResolvePass() = default;

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx,
                            const FrameGraph& graph,
                            GraphicsEncoder& encoder,
                            EventBus& eventBus) {
}

void ResolvePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& sceneBuffer = graph.getResource("sceneBuffer");
	const auto& intermediateBuffer = graph.getResource("intermediateBuffer");
	encoder.blitFramebuffer(sceneBuffer, intermediateBuffer, BlitMask::Color);
	//ctx.sceneBuffer = ctx.intermediateBuffer;
}
