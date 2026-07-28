#include "gamma.h"
#include "../../shader.h"
#include "../../renderCommand.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"

Gamma::Gamma(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Gamma::configure(const FrameGraph& graph) {
	auto shader = Shader{"models/quad.vert", "post-processing/gamma.frag"};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});
}

TextureHandle Gamma::render(
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

void Gamma::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
