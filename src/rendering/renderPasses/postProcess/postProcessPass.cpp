#include "postProcessPass.h"
#include "glad/glad.h"
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
#include "../../renderCommand.h"
#include "../../renderGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../renderContext/renderContext.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

PostProcessPass::PostProcessPass(const RenderGraph& graph, EventBus& eventBus) {
	mQuad = std::make_unique<Model::Quad>();
	mEffects = {
		std::make_shared<Bloom>("Bloom", graph, false),
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

	mRenderTargets = {&graph.getResource("ping"), &graph.getResource("pong")};
	eventBus.subscribeToEvent<PostProcessPass, GuiPostProcessEvent>(this, &PostProcessPass::onGuiUpdate);
}

PostProcessPass::~PostProcessPass() = default;

void PostProcessPass::execute(const RenderContext& ctx, const RenderGraph& graph) {
	bool toggle = false;

	uint32_t inputTex = graph.getResource("sceneBuffer").texture();
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		inputTex = effect->render(mQuad->vao(), inputTex, mRenderTargets[toggle]);
		toggle = !toggle;
	}

	mQuad->shader().activate();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const uint32_t textures[] = {inputTex};
	RenderCommand::drawQuad(mQuad->vao(), textures);
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
