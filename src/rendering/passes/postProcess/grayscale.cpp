#include "grayscale.h"
#include "../../shader.h"
#include "../../descriptorSet.h"

Grayscale::Grayscale(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Grayscale::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/grayscale.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});
}

DescriptorSet* Grayscale::render(GraphicsEncoder& encoder,
                                 DescriptorSet& dscSet,
                                 DescriptorSet& renderTargetDscSet,
                                 FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return &renderTargetDscSet;
}

void Grayscale::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
