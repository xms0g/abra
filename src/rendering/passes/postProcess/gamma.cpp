#include "gamma.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../mesh/vertexArray.h"
#include "../../models/quad.h"

Gamma::Gamma(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Gamma::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/gamma.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
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

void Gamma::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
