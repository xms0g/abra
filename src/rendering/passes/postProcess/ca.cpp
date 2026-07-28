#include "ca.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"
#include "../../../event/events/guiPostProcessEvent.hpp"

CA::CA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void CA::configure(const FrameGraph& graph) {
	auto shader = Shader{"models/quad.vert", "post-processing/ca.frag"};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});
}

TextureHandle CA::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("intensity", mIntensity);

	const uint32_t textures[] = {sceneTexture.id};
	encoder.bindMaterial({
		.flags = 0,
		.textureTarget = toGL(TextureTarget::Texture2D),
		.textures = std::span(textures)
	});

	encoder.draw({
		.vao = quad.vao(),
		.vertexCount = 6,
		.indexCount = 0
	});

	return renderTarget->texture();
}

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
