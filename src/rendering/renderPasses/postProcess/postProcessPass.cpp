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
#include "../../renderCommon.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../renderContext/renderContext.hpp"

PostProcessPass::~PostProcessPass() = default;

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
		target = std::make_unique<FrameBuffer>(static_cast<int32_t>(ctx.screen.width), static_cast<int32_t>(ctx.screen.height));
#ifdef HDR
		target->withTextureFP(GL_RGBA)
#else
		target->withTexture(GL_RGBA)
#endif
		.checkStatus();
	}
}

void PostProcessPass::execute(const RenderContext& ctx) {
	int toggle = 0;

	uint32_t inputTex = ctx.sceneBuffer->texture();
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		inputTex = effect->render(inputTex, mQuad->VAO(), toggle, mRenderTargets);
	}

	mQuad->shader().activate();
	RenderCommon::drawQuad(inputTex, mQuad->VAO());
}
