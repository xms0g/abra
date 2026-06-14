#include "grayscale.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Grayscale::Grayscale(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = ResourceManager::instance().get<Shader>("grayscale");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t Grayscale::render(
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

void Grayscale::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
