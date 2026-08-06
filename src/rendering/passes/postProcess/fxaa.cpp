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
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/fxaa.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});
}

TextureView FXAA::render(GraphicsEncoder& encoder,
                         Model::Quad& quad,
                         const TextureView sceneTexture,
                         FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);

	const TextureView textures[] = {sceneTexture};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	return renderTarget->texture();
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
