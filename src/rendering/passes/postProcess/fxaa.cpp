#include "fxaa.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"
#include "../../../event/events/guiPostProcessEvent.hpp"
#include "../../mesh/vertexArray.h"

FXAA::FXAA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void FXAA::configure(const FrameGraph& graph) {
	auto shader = Shader{"models/quad.vert", "post-processing/fxaa.frag"};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});
}

TextureHandle FXAA::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);

	const uint32_t textures[] = {sceneTexture.id};
	encoder.bindMaterial({
		.flags = 0,
		.textureTarget = toGLu(TextureTarget::Texture2D),
		.textures = std::span(textures)
	});

	encoder.draw({
		.vao = quad.vao().id(),
		.vertexCount = 6,
		.indexCount = 0
	});

	return renderTarget->texture();
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
