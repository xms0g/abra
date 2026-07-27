#include "kernel.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Kernel::Kernel(const std::string& name, const float* kernel, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
	mShader = RESOURCE_MANAGER_INSTANCE.get<Shader>("kernel");
}

void Kernel::configure(const FrameGraph& graph) {
	constexpr TextureBinding textureBindings[] = {
		{.name = "screenTexture", .slot = 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

TextureHandle Kernel::render(const uint32_t vao, const TextureHandle sceneTexture, FrameBuffer* renderTarget) const {
	renderTarget->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->bind();
	mShader->setFloatArray("kernel", mKernel, 9);

	const uint32_t textures[] = {sceneTexture.id};
	RenderCommand::drawQuad(vao, textures);

	return renderTarget->texture();
}

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
