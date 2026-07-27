#include "fxaa.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

FXAA::FXAA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("fxaa");
}

void FXAA::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

TextureHandle FXAA::render(const uint32_t vao, const TextureHandle sceneTexture, FrameBuffer* renderTarget) const {
	renderTarget->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->bind();
	mShader->setVec2("resolution", glm::vec2(renderTarget->width(), renderTarget->height()));

	const uint32_t textures[] = {sceneTexture.id};
	RenderCommand::drawQuad(vao, textures);

	return renderTarget->texture();
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
