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
#include "../../graphicsEncoder.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"
#include "../../mesh/vertexArray.h"

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
	stages.emplace_back(ShaderLoader::load("models/quad.vert"), ShaderStageType::Vertex);
	stages.emplace_back(ShaderLoader::load("models/quad.frag"), ShaderStageType::Fragment);

	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		stages,
		{{.name = "screenTexture", .slot = 0}});

	mQuad = std::make_unique<Model::Quad>();
	mRenderTargets = {&graph.getResource("ping"), &graph.getResource("pong")};
	eventBus.subscribeToEvent<PostProcessPass, GuiPostProcessEvent>(this, &PostProcessPass::onGuiUpdate);
}

void PostProcessPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	bool toggle = false;

	TextureView inputTex = graph.getResource("sceneBuffer").texture();
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		inputTex = effect->render(encoder, *mQuad, inputTex, mRenderTargets[toggle]);
		toggle = !toggle;
	}

	encoder.bindFrameBuffer();
	encoder.bindPipeline(mPipeline);

	const TextureView textures[] = {inputTex};
	encoder.bindMaterial({.textures = std::span(textures)});

	encoder.bindVertexArray(mQuad->vao().id());
	encoder.draw(6);
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
