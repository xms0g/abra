#include "terrain.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../descriptorSet.h"
#include "../graphicsEncoder.h"
#include "../command.hpp"
#include "../material/pushConstants.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderQueue.hpp"
#include "../context/renderData.hpp"
#include "../../config/configManager.h"

TerrainPass::TerrainPass() = default;

TerrainPass::~TerrainPass() = default;

void TerrainPass::configure(const RenderContext& ctx,
                            const FrameGraph& graph,
                            GraphicsEncoder& encoder,
                            EventBus& eventBus) {
	constexpr PipelinePrimitiveAssemblyState primitiveAssemblyState = {
		.topology = PrimitiveTopology::Patches,
	};

	constexpr PipelineTessellationState tessellationState = {
		.patchControlPoints = 4,
	};

	constexpr PipelineRasterizationState rasterizationState = {
		.cullMode = CullMode::Back,
		.frontFace = FrontFace::CounterClockwise,
		.polygonMode = PolygonMode::Line,
		.polygonFace = PolygonFace::FrontAndBack,
	};

	constexpr PipelineDepthStencilState depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.tessellationState = tessellationState,
		.stages = {
			{.code = ShaderLoader::load("models/terrain.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("models/terrain.frag"), .stage = ShaderStageType::Fragment},
			{.code = ShaderLoader::load("models/terrain.tesc"), .stage = ShaderStageType::TessControl},
			{.code = ShaderLoader::load("models/terrain.tese"), .stage = ShaderStageType::TessEvaluation},
		}
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{.name = "material.texture_height", .type = DescriptorType::SampledImage, .binding = 0},
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			},
		}
	};

	PipelineLayout layout = {.
		descriptorSets = {materialLayout, bufferLayout},
		.pushConstants = MaterialPushConstants::layout
	};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mCommands = &ctx.queueRegistry->get<DrawCommand>("TerrainCommands");
}

void TerrainPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto& cmd = mCommands->front();

	encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
	encoder.bindPipeline(mPipeline);
	encoder.bindDescriptorSet(ctx.renderData->material.descriptorSets[cmd.material.idx]);

	const MaterialPushConstants pushConstants = {
		.flags = cmd.material.flags,
		.heightScale = cmd.material.heightScale,
		.alphaCutoff = cmd.material.alphaCutoff,
		.color = cmd.material.color
	};
	encoder.pushConstants(&pushConstants);
	encoder.bindTransform(cmd.transform);
	encoder.bindVertexArray(cmd.mesh.vao);
	encoder.draw(cmd.mesh.vertexCount);
}
