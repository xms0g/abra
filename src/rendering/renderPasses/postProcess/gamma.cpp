#include "gamma.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

Gamma::Gamma(const std::string& name, const bool enabled) : IPostEffect(name, enabled) {
	shader = std::make_unique<Shader>("models/quad.vert", "post-processing/gamma.frag");
	shader->activate();
	shader->setInt("screenTexture", 0);
}

uint32_t Gamma::render(
	const uint32_t sceneTexture,
	const uint32_t vao,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	shader->activate();
	RenderCommon::drawQuad(sceneTexture, vao);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}
