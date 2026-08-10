#include "fxaa.h"
#include "../../shader.h"
#include "../../buffers/frameBuffer.h"
#include "../../descriptorSet.h"
#include "../../context/renderContext.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

FXAA::FXAA(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void FXAA::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/fxaa.frag"), ShaderStageType::Fragment);

	DescriptorSetLayout passLayout = {
		.bindings = {
		{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

TextureView FXAA::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("inverseResolution", glm::vec2(1.0 / renderTarget->width(), 1.0 / renderTarget->height()));
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return renderTarget->texture();
}

void FXAA::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
