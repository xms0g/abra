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
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"
#include "../../mesh/vertexArray.h"

PostProcessPass::PostProcessPass() = default;

PostProcessPass::~PostProcessPass() = default;

void PostProcessPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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

	auto shader = Shader{"models/quad.vert", "models/quad.frag"};
	mPipeline = GraphicsPipeline::createFullscreenQuadPipeline(
		shader,
		{{.name = "screenTexture", .slot = 0}});

	mEncoder = GraphicsEncoder{};
	mQuad = std::make_unique<Model::Quad>();
	mRenderTargets = {&graph.getResource("ping"), &graph.getResource("pong")};
	eventBus.subscribeToEvent<PostProcessPass, GuiPostProcessEvent>(this, &PostProcessPass::onGuiUpdate);
}

void PostProcessPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	bool toggle = false;

	TextureHandle inputTex = graph.getResource("sceneBuffer").texture();
	for (const auto& effect: mEffects) {
		if (!effect->enabled())
			continue;

		inputTex = effect->render(mEncoder, *mQuad, inputTex, mRenderTargets[toggle]);
		toggle = !toggle;
	}

	mEncoder.reset();
	mEncoder.bindFrameBuffer();
	mEncoder.bindPipeline(mPipeline);

	const uint32_t textures[] = {inputTex.id};
	mEncoder.bindMaterial({
		.flags = 0,
		.textureTarget = toGL(TextureTarget::Texture2D),
		.textures = std::span(textures)
	});

	mEncoder.draw({
		.vao = mQuad->vao().id(),
		.vertexCount = 6,
		.indexCount = 0
	});
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
