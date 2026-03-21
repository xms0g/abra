#include "grayscale.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"

Grayscale::Grayscale(const std::string& name, const bool enabled) : IPostEffect(name, enabled) {
	shader = std::make_unique<Shader>("models/quad.vert", "post-processing/grayscale.frag");
	shader->activate();
	shader->setInt("screenTexture", 0);
}

uint32_t Grayscale::render(
	const uint32_t sceneTexture,
	const uint32_t vao,
	int& toggle,
	RenderTargetType& renderTargets) const {
	renderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	shader->activate();
	RenderCommon::drawQuad(sceneTexture, vao);

	const uint32_t texture = renderTargets[toggle]->texture();
	renderTargets[toggle]->unbind();
	toggle = !toggle;
	return texture;
}
