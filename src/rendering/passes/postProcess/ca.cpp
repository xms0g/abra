#include "ca.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

CA::CA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("ca");
}

void CA::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t CA::render(const uint32_t vao, const uint32_t sceneTexture, FrameBuffer* renderTarget) const {
	renderTarget->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->bind();
	mShader->setFloat("intensity", mIntensity);

	const uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	return renderTarget->texture();
}

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
