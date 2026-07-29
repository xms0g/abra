#include "instancedOpaque.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../material/material.hpp"
#include "../buffers/vertexBuffer.h"
#include "../mesh/mesh.h"
#include "../../math/matrix.h"
#include "../../config/configManager.h"

InstancedOpaquePass::InstancedOpaquePass() = default;

InstancedOpaquePass::~InstancedOpaquePass() = default;

void InstancedOpaquePass::configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) {
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
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = false,
	};

	PipelineRenderingInfo info = {
		.primitiveAssembly = primitiveAssemblyState,
		.rasterization = rasterizationState,
		.depthStencil = depthStencilState,
		.colorBlend = colorBlendState,
		.stages = {
			{.fn = "instanced.vert", .type = ShaderStageType::Vertex},
			{.fn = "opaque.frag", .type = ShaderStageType::Fragment}
		},
		.samplers = {
			{.name = "material.texture_albedo", .slot = 0},
			{.name = "material.texture_specular", .slot = 1},
			{.name = "material.texture_normal", .slot = 2},
			{.name = "shadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot")},
			{.name = "shadowCubemap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 1},
			{.name = "persShadowMap", .slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot") + 2}
		},
		.uniforms = {
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("camera.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("camera.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("light.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("light.ubo_binding"),
			},
			{
				.name = CONFIG_MANAGER_INSTANCE.get<std::string>("shadow.block_name"),
				.binding = CONFIG_MANAGER_INSTANCE.get<uint32_t>("shadow.ubo_binding"),
			}
		}
	};

	mPipeline = GraphicsPipeline{info};
	mEncoder = GraphicsEncoder{};

	const int32_t slot = CONFIG_MANAGER_INSTANCE.get<int32_t>("shadow.texture_slot");
	mEncoder.bindTexture(graph.getResource("directional").texture(0), slot);
	mEncoder.bindTexture(graph.getResource("point").texture(0), slot + 1);
	mEncoder.bindTexture(graph.getResource("spot").texture(0), slot + 2);

	mObjects = std::span(
		ctx.queueRegistry->get<RenderInstanceGroup>("opaqueInstanced").data(),
		ctx.queueRegistry->get<RenderInstanceGroup>("opaqueInstanced").size());

	prepareInstanceBuffer(ctx.renderData->mesh.vaos);
	uploadInstanceData();
}

void InstancedOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph) {
	mEncoder.reset();
	mEncoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
	mEncoder.bindPipeline(mPipeline);

	for (const auto& object: mObjects) {
		const size_t count = object.transforms.size() / 9;

		mEncoder.bindMaterial({
			.idx = object.matBatch.materialIndex,
			.flags = ctx.renderData->material.flags[object.matBatch.materialIndex],
			.textureTarget = ctx.renderData->material.textureTargets[object.matBatch.materialIndex],
			.textures = std::span<const uint32_t>(
				ctx.renderData->material.textures.data() + object.matBatch.textureOffset,
				object.matBatch.textureCount)
		});

		for (const auto& meshIdx: object.matBatch.meshIndices) {
			mEncoder.drawInstanced({
				                       .vao = ctx.renderData->mesh.vaos[meshIdx],
				                       .vertexCount = 0,
				                       .indexCount = ctx.renderData->mesh.indexCounts[meshIdx]
			                       },
			                       count);
		}
	}
}

void InstancedOpaquePass::prepareInstanceBuffer(const std::vector<uint32_t>& vaos) {
	mVBO = std::make_unique<VertexBuffer>(DYNAMIC);

	size_t totalRequiredSize = 0;
	for (const auto& group: mObjects) {
		const size_t instanceCount = group.transforms.size() / 9;
		totalRequiredSize += instanceCount * sizeof(InstanceData);
	}

	// Allocate the full block of memory once
	mVBO->bind();
	mVBO->setData(nullptr, static_cast<uint32_t>(totalRequiredSize), 0);

	// Setup attributes now that the buffer is allocated
	uint32_t currentOffset = 0;
	for (const auto& group: mObjects) {
		for (const auto meshIdx: group.matBatch.meshIndices) {
			const uint32_t vao = vaos[meshIdx];
			Mesh::enableInstanceAttributes(vao, currentOffset);
		}
		const size_t instanceCount = group.transforms.size() / 9;
		currentOffset += static_cast<uint32_t>(instanceCount * sizeof(InstanceData));
	}

	mVBO->unbind();
}

void InstancedOpaquePass::uploadInstanceData() const {
	mVBO->bind();

	uint32_t currentOffset = 0;
	for (const auto& group: mObjects) {
		std::vector<InstanceData> gpuData;
		const size_t instanceCount = group.transforms.size() / 9;
		gpuData.reserve(instanceCount);

		for (size_t i = 0; i < group.transforms.size(); i += 9) {
			glm::vec3 pos{group.transforms[i], group.transforms[i + 1], group.transforms[i + 2]};
			glm::vec3 rot{group.transforms[i + 3], group.transforms[i + 4], group.transforms[i + 5]};
			glm::vec3 scale{group.transforms[i + 6], group.transforms[i + 7], group.transforms[i + 8]};

			glm::mat4 model = math::modelMatrix(pos, rot, scale);
			gpuData.emplace_back(model, math::normalMatrix(model));
		}

		const size_t uploadSize = gpuData.size() * sizeof(InstanceData);
		mVBO->setData(gpuData.data(), static_cast<uint32_t>(uploadSize), currentOffset);
		currentOffset += static_cast<uint32_t>(uploadSize);
	}

	mVBO->unbind();
}
