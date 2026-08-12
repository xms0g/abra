#include "postProcess.h"
#include "grayscale.h"
#include "sepia.h"
#include "bloom.h"
#include "ca.h"
#include "gamma.h"
#include "toneMapping.h"
#include "kernel.h"
#include "fxaa.h"
#include "kernels.hpp"
#include "../../shader.h"
#include "../../descriptorSet.h"
#include "../../graphicsEncoder.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

PostProcessPass::PostProcessPass() = default;

PostProcessPass::~PostProcessPass() = default;

void PostProcessPass::configure(const RenderContext& ctx,
                                const FrameGraph& graph,
                                GraphicsEncoder& encoder,
                                EventBus& eventBus) {
	mEffects = {
		std::make_shared<Bloom>("Bloom", false),
		std::make_shared<ToneMapping>("Tone Mapping", false),
		std::make_shared<Grayscale>("Grayscale", false),
		std::make_shared<Sepia>("Sepia", false),
		std::make_shared<Kernel>("Blur", blurKernel, false),
		std::make_shared<Kernel>("Edge Detection", edgeKernel, false),
		std::make_shared<Kernel>("Sharpen", sharpenKernel, false),
		std::make_shared<CA>("Chromatic Aberration", false),
		std::make_shared<Gamma>("Gamma Correction", true),
		std::make_shared<FXAA>("FXAA", false),
	};

	for (const auto& effect: mEffects) {
		effect->configure(graph);
	}

	std::vector<PipelineShaderStage> stages;
	stages.emplace_back(ShaderLoader::load("models/quad2.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("models/quad.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(stages, {
		.bindings = {
			{.name = "screenTexture", .type = DescriptorType::SampledImage, .binding = 0}
		}
	});

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mRenderTargets = {&graph.getResource(graph.getResourceID("ping")), &graph.getResource(graph.getResourceID("pong"))};

	DescriptorSet pingDescSet{};
	pingDescSet.write(0, mRenderTargets[0]->texture());

	DescriptorSet pongDescSet{};
	pongDescSet.write(0, mRenderTargets[1]->texture());

	mRenderTargetsDescSets = {pingDescSet, pongDescSet};
	eventBus.subscribeToEvent<PostProcessPass, GuiPostProcessEvent>(this, &PostProcessPass::onGuiUpdate);
}

void PostProcessPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	bool toggle = false;

	DescriptorSet sceneDescSet{};
	sceneDescSet.write(0, graph.getResource(mIndexes.sceneBuffer).texture());

	DescriptorSet* dscSet = &sceneDescSet;
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		dscSet = effect->render(encoder, *dscSet, mRenderTargetsDescSets[toggle], mRenderTargets[toggle]);
		toggle = !toggle;
	}

	encoder.bindFrameBuffer();
	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(*dscSet);
	encoder.draw(3);
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
