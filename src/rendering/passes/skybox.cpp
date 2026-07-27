#include "skybox.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"

SkyboxPass::SkyboxPass() = default;

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.depthTestEnable = true,
		.depthWriteEnable = false,
		.depthCompareOp = CompareOp::Lequal,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stage = Shader("skybox.vert", "skybox.frag"),
		.samplers = {
			{.name = "skybox", .slot = 0},
		},
		.uniforms = {}
	};

	mPipeline = GraphicsPipeline{desc};
	mEncoder = GraphicsEncoder{};
	mCommands = &ctx.queueRegistry->get<DrawCommand>("SkyboxCommands");
}

void SkyboxPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& cmd = mCommands->front();

	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);
	mEncoder.bindMaterial(cmd.material);
	mEncoder.bindTransform(cmd.transform);
	mEncoder.draw(cmd.mesh);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}
