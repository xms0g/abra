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
#include "../../renderCommand.h"
#include "../../frameGraph.h"
#include "../../buffers/frameBuffer.h"
#include "../../models/quad.h"
#include "../../context/renderContext.hpp"
#include "../../mesh/vertexArray.h"
#include "../../buffers/vertexBuffer.h"
#include "../../../event/eventBus.hpp"
#include "../../../event/events/guiPostProcessEvent.hpp"

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

	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Triangles,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Fill,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = false,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("models/quad.vert", "models/quad.frag"),
		.samplers = {
			{.name = "screenTexture", .slot = 0}
		},
		.uniforms = {}
	};

	mPipeline = GraphicsPipeline{desc};
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

		inputTex = effect->render(mQuad->vao(), inputTex, mRenderTargets[toggle]);
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
		.vao = mQuad->vao(),
		.vertexCount = 6,
		.indexCount = 0
	});
}

void PostProcessPass::onGuiUpdate(const GuiPostProcessEvent& event) {
	const auto& effect = mEffects[event.id];
	effect->updateFromEvent(event);
}
