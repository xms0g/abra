#include "kernel.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../mesh/vertexArray.h"
#include "../../models/quad.h"

Kernel::Kernel(const std::string& name, const float* kernel, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
}

void Kernel::configure(const FrameGraph& graph) {
	std::vector<ShaderStage> stages;
	stages.emplace_back("models/quad.vert", ShaderStageType::Vertex);
	stages.emplace_back("post-processing/kernel.frag", ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});
}

TextureHandle Kernel::render(
	GraphicsEncoder& encoder,
	Model::Quad& quad,
	const TextureHandle sceneTexture,
	FrameBuffer* renderTarget) {
	encoder.reset();
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("kernel", mKernel, 9);

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

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
