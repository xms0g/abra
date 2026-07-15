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
#include "../../buffers/frameBuffer.h"
#include "../../buffers/vertexBuffer.h"
#include "../../models/quad.h"
#include "../../mesh/vertexArray.h"
#include "../../renderContext/renderContext.hpp"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"
#include "../../../config/configManager.h"

PostProcessPass::PostProcessPass() = default;

PostProcessPass::~PostProcessPass() = default;

void PostProcessPass::configure(RenderContext& ctx, EventBus& eventBus) {
	mQuad = std::make_unique<Model::Quad>();
	mEffects = {
		std::make_shared<Bloom>("Bloom", ctx, false),
		std::make_shared<ToneMapping>("Tone Mapping", ctx, false),
		std::make_shared<Grayscale>("Grayscale", ctx, false),
		std::make_shared<Sepia>("Sepia", ctx, false),
		std::make_shared<Kernel>("Blur", blurKernel, ctx, false),
		std::make_shared<Kernel>("Edge Detection", edgeKernel, ctx, false),
		std::make_shared<Kernel>("Sharpen", sharpenKernel, ctx, false),
		std::make_shared<CA>("Chromatic Aberration", ctx, false),
		std::make_shared<Gamma>("Gamma Correction", ctx, true),
		std::make_shared<FXAA>("FXAA", ctx, false),
	};

	for (auto& target: mRenderTargets) {
		target = std::make_unique<FrameBuffer>(
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"),
			CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"));
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

		inputTex = effect->render(mQuad->vao(), inputTex, toggle, mRenderTargets);
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
