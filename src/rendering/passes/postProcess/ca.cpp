#include "ca.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"
#include "../../descriptorSet.h"
#include "../../../event/events/guiPostProcessEvent.hpp"

CA::CA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void CA::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/ca.frag"), ShaderStageType::Fragment);

	const DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

DescriptorSet* CA::render(GraphicsEncoder& encoder,
                          DescriptorSet& dscSet,
                          DescriptorSet& renderTargetDscSet,
                          FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("intensity", mIntensity);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return &renderTargetDscSet;
}

void CA::updateFromEventImpl(const GuiPostProcessEvent& event) {
	mIntensity = event.intensity;
}
