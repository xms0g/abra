#include "fxaa.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

FXAA::FXAA(const std::string& name, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = ctx.resourceManager->get<Shader>("fxaa");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t FXAA::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& pingPong) const {
	pingPong[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	const int32_t width = pingPong[toggle]->width();
	const int32_t height = pingPong[toggle]->height();

	mShader->activate();
	mShader->setVec2("resolution", glm::vec2(width, height));

	uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t texture = pingPong[toggle]->texture();
	pingPong[toggle]->unbind();
	toggle = !toggle;
	return texture;
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
