#include "resolve.hpp"
#include "../frameGraph.hpp"
#include "../graphicsEncoder.hpp"

ResolvePass::ResolvePass() = default;

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx,
                            const FrameGraph& graph,
                            GraphicsEncoder& encoder,
                            EventBus& eventBus) {
	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mIndexes.intermediateBuffer = graph.getResourceID("intermediateBuffer");
}

void ResolvePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& sceneBuffer = graph.getResource(mIndexes.sceneBuffer);
	const auto& intermediateBuffer = graph.getResource(mIndexes.intermediateBuffer);
	encoder.blitFramebuffer(sceneBuffer, intermediateBuffer, BlitMask::Color);
}
