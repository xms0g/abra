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

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("skybox.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("skybox.frag"), .stage = ShaderStageType::Fragment},
		},
		.samplers = {
			{.name = "skybox", .slot = 0},
		},
		.uniforms = {}
	};

	mPipeline = GraphicsPipeline{info};
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
	mEncoder.bindVertexArray(cmd.mesh.vao);
	mEncoder.draw(cmd.mesh.vertexCount);
}
