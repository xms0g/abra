#include "terrain.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../../rendering/shader.h"
#include "../../config/configManager.h"

TerrainPass::TerrainPass() = default;

TerrainPass::~TerrainPass() = default;

void TerrainPass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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

	PipelineRenderingInfo desc = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.tessellation = tessellationState,
		.stage = Shader("models/terrain.vert", "models/terrain.frag", "", "models/terrain.tcs", "models/terrain.tes"),
		.samplers = {
			{.name = "material.texture_height", .slot = 0},
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name").c_str(),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
		}
	};

	mPipeline = GraphicsPipeline{desc};
	mEncoder = GraphicsEncoder{graph};
	mObjects = &ctx.queueRegistry->get<RenderGroup>("terrain");
}

void TerrainPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();

	mEncoder.reset();
	mEncoder.bindFrameBuffer("sceneBuffer");
	mEncoder.bindPipeline(mPipeline);

	mEncoder.bindMaterial({
		.idx = matBatch.materialIndex,
		.flags = ctx.renderData->material.flags[matBatch.materialIndex],
		.textureTarget = ctx.renderData->material.textureTargets[matBatch.materialIndex],
		.heightScale = ctx.renderData->entity.heightScales[entityID],
		.textures = std::span<const uint32_t>(
			ctx.renderData->material.textures.data() + matBatch.textureOffset,
			matBatch.textureCount)
	});

	mEncoder.bindTransform({
		.model = ctx.renderData->entity.models[entityID],
		.normal = ctx.renderData->entity.normals[entityID],
	});

	mEncoder.draw({
		.vao = ctx.renderData->mesh.vaos[meshIdx],
		.vertexCount = ctx.renderData->mesh.vertexCounts[meshIdx],
		.indexCount = 0
	});
}
