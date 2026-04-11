#include "toneMapping.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

ToneMapping::ToneMapping(const std::string& name, const bool enabled)
	: IPostEffect(name, enabled) {
	shader = std::make_unique<Shader>("models/quad.vert", "post-processing/toneMapping.frag");
	shader->activate();
	shader->setInt("screenTexture", 0);
}

float& ToneMapping::exposure() {
	return mExposure;
}

uint32_t ToneMapping::render(
	const uint32_t sceneTexture,
	const uint32_t vao,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	shader->activate();
	shader->setFloat("exposure", mExposure);

	RenderCommon::drawQuad(sceneTexture, vao);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}
