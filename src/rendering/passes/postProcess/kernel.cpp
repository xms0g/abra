#include "kernel.h"
#include "../../shader.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Kernel::Kernel(const std::string& name, const float* kernel, const bool enabled)
	: BasePostEffect(name, enabled),
	  mKernel(kernel) {
}

void Kernel::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/kernel.frag"), ShaderStageType::Fragment);

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::Sampler2D, .binding = 0}
		}
	};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

TextureView Kernel::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.setUniform("kernel", mKernel, 9);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return renderTarget->texture();
}

void Kernel::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
