#include "skybox.h"
#include "glad/glad.h"
#include "../shader.h"
#include "../renderCommand.h"
#include "../graph.h"
#include "../buffers/frameBuffer.h"
#include "../material/material.hpp"
#include "../context/renderContext.hpp"
#include "../context/renderData.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderQueue.hpp"
#include "../texture/texture.h"

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
		.samples = {
			{.name = "skybox", .slot = 0},
		},
		.uniforms = {}
	};

	mPipeline = GraphicsPipeline{desc};
	mEncoder = GraphicsEncoder{graph};
	mObjects = &ctx.queueRegistry->get<RenderGroup>("skybox");
}

void SkyboxPass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	const auto& [entityID, matBatch] = mObjects->front();
	const uint32_t meshIdx = matBatch.meshIndices.front();

	mEncoder.reset();
	mEncoder.bindFrameBuffer("sceneBuffer");
	mEncoder.bindPipeline(mPipeline);

	mEncoder.bindMaterial({
		.idx = matBatch.materialIndex,
		.flags = ctx.renderData->material.flags[matBatch.materialIndex],
		.textureTarget = ctx.renderData->material.textureTargets[matBatch.materialIndex],
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
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}
