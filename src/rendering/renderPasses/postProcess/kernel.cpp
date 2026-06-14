#include "kernel.h"
#include "glad/glad.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../renderContext/renderContext.hpp"

Kernel::Kernel(const std::string& name, const float* kernel, const RenderContext& ctx, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
	mShader = ResourceManager::instance().get<Shader>("kernel");

	const std::vector<TextureBinding> textureBindings = {
		{"screenTexture", 0},
	};

	RenderCommand::setTextureUnits(textureBindings, *mShader);
}

uint32_t Kernel::render(
	const uint32_t vao,
	const uint32_t sceneTexture,
	bool& toggle,
	PingPongBuffer& renderTargets) const {
	renderTargets[toggle]->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	mShader->activate();
	mShader->setFloatArray("kernel", mKernel, 9);

	const uint32_t textures[] = {sceneTexture};
	RenderCommand::drawQuad(vao, textures);

	const uint32_t texture = renderTargets[toggle]->texture();
	toggle = !toggle;
	return texture;
}

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
