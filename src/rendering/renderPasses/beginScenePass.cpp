#include "beginScenePass.h"
#include "glad/glad.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"

BeginScenePass::~BeginScenePass() = default;

void BeginScenePass::configure(const RenderContext& ctx) {
}

void BeginScenePass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
