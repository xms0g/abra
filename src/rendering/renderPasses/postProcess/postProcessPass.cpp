#include "postProcessPass.h"
#include "grayscale.h"
#include "sepia.h"
#include "bloom.h"
#include "ca.h"
#include "gamma.h"
#include "toneMapping.h"
#include "kernel.h"
#include "fxaa.h"
#include "../../shader.h"
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../renderContext/renderContext.hpp"

PostProcessPass::~PostProcessPass() {
	for (const auto& target: mRenderTargets) {
		delete target;
	}
}

std::vector<std::shared_ptr<IPostEffect>>& PostProcessPass::effects() {
	return mEffects;
}

void PostProcessPass::configure(const RenderContext& ctx) {
	mQuad = std::make_unique<Models::Quad>();

	mEffects = {
		std::make_shared<Bloom>("Bloom", ctx.screen.width, ctx.screen.height, false),
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

	for (auto& target: mRenderTargets) {
		target = new FrameBuffer(static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
#ifdef HDR
		target->withTextureFP(16, true)
#else
		target->withTexture()
#endif
		.checkStatus();
	}
}

void PostProcessPass::execute(const RenderContext& ctx) {
	uint32_t inputTex = ctx.sceneBuffer->texture();
	int toggle = 0;

	for (const auto& effect: mEffects) {
		if (!effect->enabled()) continue;

		inputTex = effect->render(inputTex, mQuad->VAO(), toggle, mRenderTargets);
	}

	mQuad->shader().activate();
	RenderCommon::drawQuad(inputTex, mQuad->VAO());
}
