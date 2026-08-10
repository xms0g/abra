#include "gamma.h"
#include "../../shader.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Gamma::Gamma(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Gamma::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/gamma.frag"), ShaderStageType::Fragment);

	const DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

TextureView Gamma::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return renderTarget->texture();
}

void Gamma::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
