#include "sepia.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	shader = std::make_unique<Shader>("models/quad.vert", "post-processing/sepia.frag");
	shader->activate();
	shader->setInt("screenTexture", 0);
}

uint32_t Sepia::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	shader->activate();
	RenderCommon::drawQuad(vao, sceneTexture);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
