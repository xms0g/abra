#include "fxaa.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../descriptorSet.h"
#include "../../../event/events/guiPostProcessEvent.hpp"

FXAA::FXAA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void FXAA::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/fxaa.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});
}

DescriptorSet* FXAA::render(GraphicsEncoder& encoder,
                            DescriptorSet& dscSet,
                            DescriptorSet& renderTargetDscSet,
                            FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("inverseResolution", glm::vec2(1.0 / renderTarget->width(), 1.0 / renderTarget->height()));
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return &renderTargetDscSet;
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
