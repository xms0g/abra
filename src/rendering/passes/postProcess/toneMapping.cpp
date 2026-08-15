#include "toneMapping.h"
#include "../../shader.h"
#include "../../descriptorSet.h"

ToneMapping::ToneMapping(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void ToneMapping::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/toneMapping.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});
}

DescriptorSet* ToneMapping::render(GraphicsEncoder& encoder,
                                   DescriptorSet& dscSet,
                                   DescriptorSet& renderTargetDscSet,
                                   FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("exposure", mExposure);
	encoder.bindDescriptorSet(mPipeline.layout().descriptorSets[0], dscSet);
	encoder.draw(3);

	return &renderTargetDscSet;
}

void ToneMapping::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mExposure = event.exposure;
}
