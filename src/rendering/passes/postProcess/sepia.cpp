#include "sepia.h"
#include "../../shader.h"
#include "../../descriptorSet.h"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Sepia::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/sepia.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});
}

DescriptorSet* Sepia::render(GraphicsEncoder& encoder,
                             DescriptorSet& dscSet,
                             DescriptorSet& renderTargetDscSet,
                             FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(mPipeline.layout().descriptorSets[0], dscSet);
	encoder.draw(3);

	return &renderTargetDscSet;
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
