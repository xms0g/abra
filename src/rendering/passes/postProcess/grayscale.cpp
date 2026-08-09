#include "grayscale.h"
#include "../../shader.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Grayscale::Grayscale(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Grayscale::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/grayscale.frag"), ShaderStageType::Fragment);

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::Sampler2D, .binding = 0}
		}
	};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

TextureView Grayscale::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return renderTarget->texture();
}

void Grayscale::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
