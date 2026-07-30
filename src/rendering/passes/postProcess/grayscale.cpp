#include "grayscale.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../mesh/vertexArray.h"
#include "../../models/quad.h"

Grayscale::Grayscale(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Grayscale::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/grayscale.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
{{.name = "screenTexture", .slot = 0}});
}

TextureHandle Grayscale::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);
	encoder.setViewport({.x = 0, .y = 0, .width = renderTarget->width(), .height = renderTarget->height()});

	encoder.bindPipeline(mPipeline);

	const TextureHandle textures[] = {{.id = sceneTexture.id, .target = TextureTarget::Texture2D}};
	encoder.bindMaterial({
		.flags = 0,
		.textures = std::span(textures)
	});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	return renderTarget->texture();
}

void Grayscale::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
