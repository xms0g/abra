#include "kernel.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

Kernel::Kernel(const std::string& name, const float* kernel, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
	shader = std::make_unique<Shader>("models/quad.vert", "post-processing/kernel.frag");
	shader->activate();
	shader->setInt("screenTexture", 0);
}

uint32_t Kernel::render(
	const uint32_t sceneTexture,
	const uint32_t vao,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	shader->activate();
	shader->setFloatArray("kernel", mKernel, 9);

	RenderCommon::drawQuad(sceneTexture, vao);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
