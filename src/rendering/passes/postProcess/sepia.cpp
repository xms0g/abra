#include "sepia.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("sepia");
}

void Sepia::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

TextureHandle Sepia::render(const uint32_t vao, const TextureHandle sceneTexture, FrameBuffer* renderTarget) const {
	renderTarget->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->bind();

	const uint32_t textures[] = {sceneTexture.id};
	RenderCommand::drawQuad(vao, textures);

	return renderTarget->texture();
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
