#include "beginScenePass.h"
#include "glad/glad.h"
#include "../buffers/frameBuffer.h"
#include "../renderContext/renderContext.hpp"

BeginScenePass::~BeginScenePass() = default;

void BeginScenePass::configure(const RenderContext& ctx) {
}

void BeginScenePass::execute(const RenderContext& ctx) {
	ctx.sceneBuffer->bind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ctx.sceneBuffer->unbind();
}
