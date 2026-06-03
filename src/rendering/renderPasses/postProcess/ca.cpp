#include "ca.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

CA::CA(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = ctx.resourceManager->get<Shader>("ca");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	RenderCommand::bindTextures(textureBindings, mShader);

}

uint32_t CA::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	mShader->setFloat("intensity", mIntensity);

	RenderCommand::drawQuad(vao, sceneTexture);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
