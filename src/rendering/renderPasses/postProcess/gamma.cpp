#include "gamma.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Gamma::Gamma(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = rm.get<Shader>("gamma");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t Gamma::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& renderTargets) const {
	renderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();

	const uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t texture = renderTargets[toggle]->texture();
	toggle = !toggle;
	return texture;
}

void Gamma::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
