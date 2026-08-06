#include "ca.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

CA::CA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void CA::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/ca.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});
}

TextureView CA::render(GraphicsEncoder& encoder, const TextureView sceneTexture, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("intensity", mIntensity);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.draw(3);

	return renderTarget->texture();
}

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
