#include "sepia.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Sepia::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/sepia.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .type = DescriptorType::Sampler2D, .binding = 0}});
}

TextureView Sepia::render(GraphicsEncoder& encoder, const TextureView sceneTexture, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.draw(3);

	return renderTarget->texture();
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
