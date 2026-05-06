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
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

PostProcessPass::~PostProcessPass() = default;

void PostProcessPass::configure(const RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Models::Quad>();

	mEffects.emplace_back(std::make_unique<Bloom>("Bloom", ctx.screen.width, ctx.screen.height, false));
	mEffects.emplace_back(std::make_unique<ToneMapping>("Tone Mapping", false));
	mEffects.emplace_back(std::make_unique<Grayscale>("Grayscale", false));
	mEffects.emplace_back(std::make_unique<Sepia>("Sepia", false));
	mEffects.emplace_back(std::make_unique<Kernel>("Blur", blurKernel, false));
	mEffects.emplace_back(std::make_unique<Kernel>("Edge Detection", edgeKernel, false));
	mEffects.emplace_back(std::make_unique<Kernel>("Sharpen", sharpenKernel, false));
	mEffects.emplace_back(std::make_unique<CA>("Chromatic Aberration", false));
	mEffects.emplace_back(std::make_unique<Gamma>("Gamma Correction", true));
	mEffects.emplace_back(std::make_unique<FXAA>("FXAA", false));

	for (auto& target: mPingPong) {
		target = std::make_unique<FrameBuffer>(
			static_cast<int32_t>(ctx.screen.width),
			static_cast<int32_t>(ctx.screen.height));
#ifdef HDR
		target->withTextureFP(GL_RGBA)
#else
		target->withTexture(GL_RGBA)
#endif
				.checkStatus();
	}

	eventBus.subscribeToEvent<PostProcessPass, GuiPostProcessEvent>(this, &PostProcessPass::onGuiUpdate);

}

void PostProcessPass::execute(const RenderContext& ctx) {
	bool toggle = false;

	uint32_t inputTex = ctx.sceneBuffer->texture();
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		inputTex = effect->render(inputTex, mQuad->VAO(), toggle, mPingPong);
	}

	mQuad->shader().activate();
	RenderCommon::drawQuad(inputTex, mQuad->VAO());
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
