#include "toneMapping.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

ToneMapping::ToneMapping(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("toneMapping");

	constexpr TextureBinding textureBindings[] = {
		{"screenTexture", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t ToneMapping::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& renderTargets) const {
	renderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	mShader->setFloat("exposure", mExposure);

	const uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t texture = renderTargets[toggle]->texture();
	toggle = !toggle;
	return texture;
}

void ToneMapping::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mExposure = event.exposure;
}
