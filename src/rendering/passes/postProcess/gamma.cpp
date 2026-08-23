#include "gamma.hpp"
#include "../../shader.hpp"
#include "../../graphicsEncoder.hpp"
#include "../../frameGraph.hpp"
#include "../../descriptorSet.hpp"

Gamma::Gamma(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Gamma::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/gamma.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});
}

DescriptorSet* Gamma::render(GraphicsEncoder& encoder,
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

void Gamma::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
