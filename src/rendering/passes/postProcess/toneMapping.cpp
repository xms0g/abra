#include "toneMapping.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

ToneMapping::ToneMapping(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void ToneMapping::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/toneMapping.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});
}

TextureView ToneMapping::render(GraphicsEncoder& encoder, const TextureView sceneTexture, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("exposure", mExposure);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.draw(3);

	return renderTarget->texture();
}

void ToneMapping::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mExposure = event.exposure;
}
