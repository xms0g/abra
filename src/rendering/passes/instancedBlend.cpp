#include "instancedBlend.h"
#include "../frameGraph.h"
#include "../shader.h"
#include "../descriptorSet.h"
#include "../graphicsEncoder.h"
#include "../context/renderContext.hpp"
#include "../context/renderGroup.hpp"
#include "../context/renderData.hpp"
#include "../context/renderQueue.hpp"
#include "../material/material.hpp"
#include "../buffers/vertexBuffer.h"
#include "../mesh/mesh.h"
#include "../../math/matrix.h"
#include "../../config/configManager.h"

InstancedBlendPass::InstancedBlendPass() = default;

InstancedBlendPass::~InstancedBlendPass() = default;

void InstancedBlendPass::configure(const RenderContext& ctx,
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
		.depthCompareOp = CompareOp::Less,
	};

	constexpr PipelineColorBlendState colorBlendState = {
		.blendEnable = true,
		.srcColorBlendFactor = BlendFactor::SrcAlpha,
		.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha,
		.colorBlendOp = BlendOp::Add,
		.srcAlphaBlendFactor = BlendFactor::One,
		.dstAlphaBlendFactor = BlendFactor::Zero,
		.alphaBlendOp = BlendOp::Add,
		.colorWriteMask = ColorComponent::Red | ColorComponent::Green | ColorComponent::Blue | ColorComponent::Alpha,
	};

	PipelineRenderingInfo info = {
		.primitiveAssemblyState = primitiveAssemblyState,
		.rasterizationState = rasterizationState,
		.depthStencilState = depthStencilState,
		.colorBlendState = colorBlendState,
		.stages = {
			{.code = ShaderLoader::load("instanced.vert"), .stage = ShaderStageType::Vertex},
			{.code = ShaderLoader::load("blend.frag"), .stage = ShaderStageType::Fragment},
		}
	};

	DescriptorSetLayout materialLayout = {
		.bindings = {
			{.name = "material.texture_albedo", .type = DescriptorType::Sampler2D, .binding = 0},
			{.name = "material.texture_specular", .type = DescriptorType::Sampler2D, .binding = 1},
			{.name = "material.texture_normal", .type = DescriptorType::Sampler2D, .binding = 2},
			{.name = "material.texture_height", .type = DescriptorType::Sampler2D, .binding = 3},
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
				.type = DescriptorType::Sampler2D,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot")
			},
			{
				.name = "shadowCubemap",
				.type = DescriptorType::SamplerCubeArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1
			},
			{
				.name = "persShadowMap",
				.type = DescriptorType::Sampler2DArray,
				.binding = CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2
			},
		}
	};

	PipelineLayout layout = {.descriptorSets = {materialLayout, bufferLayout, passLayout}};
	GraphicsPipelineCreateInfo createInfo = {.rendering = info, .layout = layout};
	mPipeline = GraphicsPipeline{createInfo};

	DescriptorSet frameSet{};
	frameSet.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot"),
				graph.getResource("directional").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 1,
				graph.getResource("point").texture())
			.write(
				CONFIG_MANAGER.get<int32_t>("shadow.map.slot") + 2,
				graph.getResource("spot").texture());

	encoder.bindDescriptorSet(frameSet);

	mObjects = std::span(
		ctx.queueRegistry->get<RenderInstanceGroup>("blendInstanced").data(),
		ctx.queueRegistry->get<RenderInstanceGroup>("blendInstanced").size());

	prepareInstanceBuffer(ctx.renderData->mesh.vaos);
	uploadInstanceData();
}

void InstancedBlendPass::execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) {
	const auto pipelineCullMode = mPipeline.rasterizationState().cullMode;

	for (const auto& object: mObjects) {
		const size_t count = object.transforms.size() / 9;

		encoder.bindFrameBuffer(graph.getResource("sceneBuffer"));
		encoder.bindPipeline(mPipeline);
		encoder.bindDescriptorSet(ctx.renderData->material.descriptorSets[object.matBatch.materialIndex]);
		encoder.pushConstants({
			.idx = object.matBatch.materialIndex,
			.flags = ctx.renderData->material.flags[object.matBatch.materialIndex]
		});
		encoder.setCullMode(object.matBatch.materialFlags & TWOSIDED ? CullMode::None : pipelineCullMode);

		for (const auto& meshIdx: object.matBatch.meshIndices) {
			encoder.bindVertexArray(ctx.renderData->mesh.vaos[meshIdx]);
			encoder.drawInstanced(ctx.renderData->mesh.indexCounts[meshIdx], count);
		}
	}
}

void InstancedBlendPass::prepareInstanceBuffer(const std::vector<uint32_t>& vaos) {
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

void InstancedBlendPass::uploadInstanceData() const {
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
