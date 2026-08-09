#include "bloom.h"
#include "../../shader.h"
#include "../../frameGraph.h"
#include "../../descriptorSet.h"
#include "../../buffers/frameBuffer.h"
#include "../../context/renderContext.hpp"

Bloom::Bloom(const std::string& name, const bool enabled)
	: BasePostEffect(name, enabled) {
}

Bloom::~Bloom() = default;

void Bloom::configure(const FrameGraph& graph) {
	std::vector<PipelineShaderStage> stages0;
	stages0.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages0.emplace_back(ShaderLoader::load("post-processing/bloom/brightFilter.frag"), ShaderStageType::Fragment);

	const DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::Sampler2D, .binding = 0}
		}
	};

	mPipelines[0] = GraphicsPipeline::createFullscreenQuadPipeline(stages0, passLayout);

	std::vector<PipelineShaderStage> stages1;
	stages1.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages1.emplace_back(ShaderLoader::load("post-processing/bloom/blur.frag"), ShaderStageType::Fragment);

	mPipelines[1] = GraphicsPipeline::createFullscreenQuadPipeline(stages1, passLayout);

	std::vector<PipelineShaderStage> stages2;
	stages2.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages2.emplace_back(ShaderLoader::load("post-processing/bloom/combine.frag"), ShaderStageType::Fragment);

	const DescriptorSetLayout combineLayout = {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::Sampler2D, .binding = 0},
			{.name = "bloomBlur", .type = DescriptorType::Sampler2D, .binding = 1}
		}
	};

	mPipelines[2] = GraphicsPipeline::createFullscreenQuadPipeline(stages2, combineLayout);
	mRenderTargets = {&graph.getResource("bloomPing"), &graph.getResource("bloomPong")};
}

TextureView Bloom::render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) {
	(void) renderTarget;
	bool toggle = false;

	TextureView inputTex = brightFilterPass(encoder, dscSet, toggle);

	DescriptorSet set;
	set.write(0, inputTex);

	inputTex = blurPass(encoder, set, toggle);

	dscSet.write(1, inputTex);

	inputTex = combinePass(encoder, dscSet, toggle);

	return inputTex;
}

void Bloom::updateFromEventImpl(const GuiPostProcessEvent& event) {
}

TextureView Bloom::brightFilterPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle) {
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[0]);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	const FrameBuffer* renderTarget = mRenderTargets[toggle];
	toggle = !toggle;

	return renderTarget->texture();
}

TextureView Bloom::blurPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle) {
	TextureView outTex;

	encoder.bindPipeline(mPipelines[1]);
	for (int i = 0; i < 10; ++i) {
		encoder.bindFrameBuffer(*mRenderTargets[toggle]);
		encoder.setUniform("horizontal", (i & 1) == 0);

		DescriptorSet set;
		set.write(0, outTex);

		if (i == 0)
			encoder.bindDescriptorSet(dscSet);
		else
			encoder.bindDescriptorSet(set);

		encoder.draw(3);

		outTex = mRenderTargets[toggle]->texture();
		toggle = !toggle;
	}

	return outTex;
}

TextureView Bloom::combinePass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, const bool& toggle) {
	encoder.bindFrameBuffer(*mRenderTargets[toggle]);
	//encoder.clearFrameBuffer(ClearMask::Color);

	encoder.bindPipeline(mPipelines[2]);
	encoder.bindDescriptorSet(dscSet);
	encoder.draw(3);

	return mRenderTargets[toggle]->texture();
}
