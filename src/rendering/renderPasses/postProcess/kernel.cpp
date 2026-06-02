#include "kernel.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Kernel::Kernel(const std::string& name, const float* kernel, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
	mShader = ctx.resourceManager->get<Shader>("kernel");
	mShader->activate();
	mShader->setInt("screenTexture", 0);
}

uint32_t Kernel::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	mShader->setFloatArray("kernel", mKernel, 9);

	RenderCommon::drawQuad(vao, sceneTexture);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
