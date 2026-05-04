#include "resolvePass.h"
#include "glad/glad.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"

ResolvePass::~ResolvePass() = default;

void ResolvePass::configure(const RenderContext& ctx, EventBus& eventBus) {
}

void ResolvePass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bindForRead();
	ctx.intermediateBuffer->bindForDraw();
	glBlitFramebuffer(0, 0, ctx.sceneBuffer->width(), ctx.sceneBuffer->height(),
					  0, 0, ctx.intermediateBuffer->width(), ctx.intermediateBuffer->height(),
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	ctx.sceneBuffer = ctx.intermediateBuffer;
}
