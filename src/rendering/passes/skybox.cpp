#include "skybox.h"
#include "../shader.h"
#include "../frameGraph.h"
#include "../descriptorSet.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../pushConstants/transformPushConstants.hpp"

SkyboxPass::SkyboxPass() = default;

SkyboxPass::~SkyboxPass() = default;

void SkyboxPass::configure(const RenderContext& ctx,
                           const FrameGraph& graph,
                           GraphicsEncoder& encoder,
                           EventBus& eventBus) {
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
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			{.name = "skybox", .type = DescriptorType::SampledImage, .binding = 0},
		}
	};

	auto transformPushConstantsLayout = TransformPushConstants::layout;
	transformPushConstantsLayout.baseOffset = 0;

	PipelineLayout layout = {
		.descriptorSets = {passLayout},
		.pushConstants = {transformPushConstantsLayout}
	};

	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mCommands = &ctx.queueRegistry->get<DrawCommand>("SkyboxCommands");
}

void SkyboxPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& cmd = mCommands->front();
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;
	const auto& materialLayout = mPipeline.layout().descriptorSets[0];

	encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(materialLayout, ctx.renderData->material.descriptorSets[cmd.material.idx]);

	const TransformPushConstants pushConstants = {
		.model = cmd.transform.model,
		.normal = cmd.transform.normal
	};

	encoder.pushConstants(&pushConstants);
	encoder.setCullMode(cmd.material.flags & TWOSIDED ? CullMode::None : pipelineCullMode);
	encoder.bindVertexArray(cmd.mesh.vao);
	encoder.draw(cmd.mesh.vertexCount);
}
