#include "sepia.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../mesh/vertexArray.h"
#include "../../models/quad.h"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Sepia::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/sepia.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
{{.name = "screenTexture", .slot = 0}});
}

TextureView Sepia::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureView sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);

	const TextureView textures[] = {{.id = sceneTexture.id, .target = TextureTarget::Texture2D}};
	encoder.bindMaterial({
		.flags = 0,
		.textures = std::span(textures)
	});

	encoder.bindVertexArray(quad.vao().id());
	encoder.draw(6);

	return renderTarget->texture();
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
