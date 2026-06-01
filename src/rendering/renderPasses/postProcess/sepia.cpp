#include "sepia.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Sepia::Sepia(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = ctx.resourceManager->getShader("sepia");
	mShader->activate();
	mShader->setInt("screenTexture", 0);
}

uint32_t Sepia::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	RenderCommon::drawQuad(vao, sceneTexture);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
