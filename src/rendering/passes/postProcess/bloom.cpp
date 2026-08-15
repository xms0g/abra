#include "bloom.h"
#include "../../shader.h"
#include "../../frameGraph.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"

Bloom::Bloom(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

Bloom::~Bloom() = default;

void Bloom::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages0;
	stages0.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages0.emplace_back(ShaderLoader::load("post-processing/bloom/brightFilter.frag"), ShaderStageType::Fragment);


	mPipelines[0] = GraphicsPipeline::createFullscreenQuadPipeline(stages0, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});

	std::vector<PipelineShaderStage> stages1;
	stages1.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages1.emplace_back(ShaderLoader::load("post-processing/bloom/blur.frag"), ShaderStageType::Fragment);

	mPipelines[1] = GraphicsPipeline::createFullscreenQuadPipeline(stages1, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});

	std::vector<PipelineShaderStage> stages2;
	stages2.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages2.emplace_back(ShaderLoader::load("post-processing/bloom/combine.frag"), ShaderStageType::Fragment);


	mPipelines[2] = GraphicsPipeline::createFullscreenQuadPipeline(stages2, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0},
			{.name = "bloomBlur", .type = DescriptorType::SampledImage, .binding = 1}
		}
	});
	mRenderTargets = {&graph.getResource(graph.getResourceID("bloomPing")), &graph.getResource(graph.getResourceID("bloomPong"))};

	DescriptorSet pingDescSet{};
	pingDescSet.write(mRenderTargets[0]->texture());

	DescriptorSet pongDescSet{};
	pongDescSet.write(mRenderTargets[1]->texture());

	mRenderTargetsDescSets = {pingDescSet, pongDescSet};
}

DescriptorSet* Bloom::render(GraphicsEncoder& encoder,
                             DescriptorSet& dscSet,
                             DescriptorSet& renderTargetDscSet,
                             FrameBuffer* renderTarget) {
	(void) renderTarget;
	(void) renderTargetDscSet;
	bool toggle = false;

	DescriptorSet* inputDescSet = brightFilterPass(encoder, dscSet, toggle);
	inputDescSet = blurPass(encoder, *inputDescSet, toggle);
	inputDescSet = combinePass(encoder, dscSet, *inputDescSet, toggle);

	return inputDescSet;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

DescriptorSet* Bloom::brightFilterPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle) {
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[0]);
	encoder.bindDescriptorSet(mPipelines[0].layout().descriptorSets[0], dscSet);
	encoder.draw(3);

	DescriptorSet* renderTargetDescSet = &mRenderTargetsDescSets[toggle];
	toggle = !toggle;

	return renderTargetDescSet;
}

DescriptorSet* Bloom::blurPass(GraphicsEncoder& encoder, DescriptorSet& dscSet, bool& toggle) {
	DescriptorSet* renderTargetDescSet = &dscSet;

	encoder.bindPipeline(mPipelines[1]);
	for (int i = 0; i < 10; ++i) {
		encoder.bindFrameBuffer(*mRenderTargets[toggle]);
		encoder.setUniform("horizontal", (i & 1) == 0);
		encoder.bindDescriptorSet(mPipelines[1].layout().descriptorSets[0], *renderTargetDescSet);
		encoder.draw(3);

		renderTargetDescSet = &mRenderTargetsDescSets[toggle];
		toggle = !toggle;
	}

	return renderTargetDescSet;
}

DescriptorSet* Bloom::combinePass(GraphicsEncoder& encoder,
                                  const DescriptorSet& dscSet,
                                  const DescriptorSet& blurDscSet,
                                  const bool& toggle) {
	DescriptorSet combineDescSet{};
	combineDescSet.write(dscSet[0]);
	combineDescSet.write(blurDscSet[0]);

	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	//encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[2]);
	encoder.bindDescriptorSet(mPipelines[2].layout().descriptorSets[0], combineDescSet);
	encoder.draw(3);

	return &mRenderTargetsDescSets[toggle];
}
