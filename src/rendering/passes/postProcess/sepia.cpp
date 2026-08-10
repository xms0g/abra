#include "sepia.h"
#include "../../shader.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"

Sepia::Sepia(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

void Sepia::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("post-processing/sepia.frag"), ShaderStageType::Fragment);

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, passLayout);
}

TextureView Sepia::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	encoder.bindFrameBuffer(*renderTarget);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return renderTarget->texture();
}

void Sepia::updateFromEventImpl(const GuiPostProcessEvent& event) {
}
