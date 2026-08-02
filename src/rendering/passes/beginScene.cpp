#include "beginScene.h"
#include "../frameGraph.h"
#include "../graphicsEncoder.h"

BeginScenePass::BeginScenePass() = default;

BeginScenePass::~BeginScenePass() = default;

void BeginScenePass::configure(
	const RenderContext& ctx,
	const FrameGraph& graph,
	GraphicsEncoder& encoder,
	EventBus& eventBus) {
}

void BeginScenePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	auto& frameBuffer = graph.getResource("sceneBuffer");

	encoder.beginRendering({
		.frameBuffer = frameBuffer,
		.clearColor = true,
		.clearDepth = true,
		.viewport = {.x = 0, .y = 0, .width = frameBuffer.width(), .height = frameBuffer.height()}
	});
}
