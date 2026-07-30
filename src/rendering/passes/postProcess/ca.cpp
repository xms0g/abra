#include "ca.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../models/quad.h"
#include "../../../event/events/guiPostProcessEvent.hpp"
#include "../../mesh/vertexArray.h"

CA::CA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void CA::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/ca.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
{{.name = "screenTexture", .slot = 0}});
}

TextureHandle CA::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("intensity", mIntensity);

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

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
