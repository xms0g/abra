#include "toneMapping.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

ToneMapping::ToneMapping(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("toneMapping");
}

void ToneMapping::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

TextureHandle ToneMapping::render(const uint32_t vao, const TextureHandle sceneTexture, FrameBuffer* renderTarget) const {
	renderTarget->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->bind();
	mShader->setFloat("exposure", mExposure);

	const uint32_t textures[] = {sceneTexture.id};
	RenderCommand::drawQuad(vao, textures);

	return renderTarget->texture();
}

void ToneMapping::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mExposure = event.exposure;
}
