#include "grayscale.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Grayscale::Grayscale(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Grayscale::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/grayscale.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});
}

TextureView Grayscale::render(GraphicsEncoder& encoder, const TextureView sceneTexture, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.draw(3);

	return renderTarget->texture();
}

void Grayscale::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
