#include "beginScene.h"
#include "../frameGraph.h"

BeginScenePass::BeginScenePass() = default;

BeginScenePass::~BeginScenePass() = default;

void BeginScenePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
}

void BeginScenePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	auto& framebuffer = graph.getResource("sceneBuffer");

	mEncoder.beginRendering({
		.framebuffer = framebuffer,
		.clearColor = true,
		.clearDepth = true,
		.viewport = {.x = 0, .y = 0, .width = framebuffer.width(), .height = framebuffer.height()}
	});
}
