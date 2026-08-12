#include "instancedOpaque.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../descriptorSet.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../material/material.hpp"
#include "../material/pushConstants.hpp"
#include "../buffers/vertexBuffer.h"
#include "../mesh/mesh.h"
#include "../../math/matrix.h"
#include "../../config/configManager.h"

InstancedOpaquePass::InstancedOpaquePass() = default;

InstancedOpaquePass::~InstancedOpaquePass() = default;

void InstancedOpaquePass::configure(const RenderContext& ctx,
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
		.stages = {
			{.code = ShaderLoader::load("instanced.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("opaque.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{.name = "material.texture_albedo", .type = DescriptorType::SampledImage, .binding = 0},
			{.name = "material.texture_specular", .type = DescriptorType::SampledImage, .binding = 1},
			{.name = "material.texture_normal", .type = DescriptorType::SampledImage, .binding = 2},
			{.name = "material.texture_height", .type = DescriptorType::SampledImage, .binding = 3},
		}
	};

	DescriptorSetLayout bufferLayout = {
		.bindings = {
			{
				.name = CONFIG_MANAGER.get<std::string>("camera.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("camera.ubo.binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("light.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("light.ubo.binding"),
			},
			{
				.name = CONFIG_MANAGER.get<std::string>("shadow.ubo.blockName"),
				.type = DescriptorType::UniformBuffer,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.ubo.binding"),
			}
		}
	};

	DescriptorSetLayout passLayout = {
		.bindings = {
			{
				.name = "shadowMap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot")
			},
			{
				.name = "shadowCubemap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1
			},
			{
				.name = "persShadowMap",
				.type = DescriptorType::SampledImage,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2
			},
		}
	};

	PushConstantLayout pushConstantLayout = {
		.constants = {
			{
				{
					.name = "material.flags", .offset = offsetof(MaterialPushConstants, flags),
					.type = PushConstantType::UInt
				},
				{
					.name = "material.heightScale", .offset = offsetof(MaterialPushConstants, heightScale),
					.type = PushConstantType::Float
				},
				{
					.name = "material.alphaCutoff", .offset = offsetof(MaterialPushConstants, alphaCutoff),
					.type = PushConstantType::Float
				},
				{
					.name = "material.color", .offset = offsetof(MaterialPushConstants, color),
					.type = PushConstantType::Vec3
				}
			}
		},
		.count = 4
	};

	PipelineLayout layout = {
		.descriptorSets = {materialLayout, bufferLayout, passLayout},
		.pushConstants = pushConstantLayout
	};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	DescriptorSet frameSet{};
	frameSet.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot"),
				graph.getResource(mIndexes.directional).texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1,
				graph.getResource(mIndexes.point).texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2,
				graph.getResource(mIndexes.spot).texture());

	encoder.bindDescriptorSet(frameSet);

	mIndexes.sceneBuffer = graph.getResourceID("sceneBuffer");
	mIndexes.directional = graph.getResourceID("directional");
	mIndexes.point = graph.getResourceID("point");
	mIndexes.spot = graph.getResourceID("spot");

	mObjects = std::span(
		ctx.queueRegistry->get<RenderInstanceGroup>("opaqueInstanced").data(),
		ctx.queueRegistry->get<RenderInstanceGroup>("opaqueInstanced").size());

	prepareInstanceBuffer(ctx.renderData->mesh.vaos);
	uploadInstanceData();
}

void InstancedOpaquePass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;

	for (const auto& object: mObjects) {
		const size_t count = object.transforms.size() / 9;

		encoder.bindFrameBuffer(graph.getResource(mIndexes.sceneBuffer));
		encoder.bindPipeline(mPipeline);
		encoder.bindDescriptorSet(ctx.renderData->material.descriptorSets[object.matBatch.materialIndex]);

		const MaterialPushConstants pushConstants = {
			.flags = object.matBatch.materialFlags,
			.heightScale = ctx.renderData->entity.heightScales[object.entityID],
			.alphaCutoff = ctx.renderData->material.alphaCutoffs[object.matBatch.materialIndex],
			.color = ctx.renderData->material.colors[object.matBatch.materialIndex]
		};
		encoder.pushConstants(&pushConstants);
		encoder.setCullMode(object.matBatch.materialFlags & TWOSIDED ? CullMode::None : pipelineCullMode);

		for (const auto& meshIdx: object.matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawInstanced(ctx.renderData->mesh.indexCounts[meshIdx], count);
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
	mVBO->setData(nullptr, static_cast<int32_t>(totalRequiredSize), 0);

	// Setup attributes now that the buffer is allocated
	int32_t currentOffset = 0;
	for (const auto& group: mObjects) {
		for (const auto meshIdx: group.matBatch.meshIndices) {
			const int32_t vao = vaos[meshIdx];
			Mesh::enableInstanceAttributes(vao, currentOffset);
		}
		const size_t instanceCount = group.transforms.size() / 9;
		currentOffset += static_cast<int32_t>(instanceCount * sizeof(InstanceData));
	}

	mVBO->unbind();
}

void InstancedOpaquePass::uploadInstanceData() const {
	mVBO->bind();

	int32_t currentOffset = 0;
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
		mVBO->setData(gpuData.data(), static_cast<int32_t>(uploadSize), currentOffset);
		currentOffset += static_cast<int32_t>(uploadSize);
	}

	mVBO->unbind();
}
