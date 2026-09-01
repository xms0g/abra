#include "batcher.hpp"
#include "context/renderData.hpp"
#include "context/renderGroup.hpp"
#include "context/renderFlag.hpp"
#include "context/renderQueue.hpp"
#include "mesh/mesh.hpp"
#include "mesh/vertex.hpp"
#include "mesh/vertexArray.hpp"
#include "../math/boundingVolume.hpp"
#include "../ECS/registry.hpp"
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"

void Batcher::build(RenderData& renderData, QueueRegistry& queueRegistry, const std::vector<Entity>& entities) {
	for (const auto& entity: entities) {
		batch(entity, renderData, queueRegistry);
	}
}

void Batcher::batch(const Entity& entity, RenderData& renderData, QueueRegistry& queueRegistry) {
	batchTransform(entity, renderData);
	batchBV(entity, renderData);
	batchDebugMode(entity, renderData);

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		std::vector<uint32_t> meshIndices = batchMeshes(renderData, meshes);
		MaterialBatch matBatch = batchMaterial(matID, entity, renderData);
		matBatch.meshIndices = std::move(meshIndices);

		enqueueRenderGroup(entity, queueRegistry, matBatch);
	}
}

void Batcher::batchTransform(const Entity& entity, RenderData& renderData) {
	const auto& transform = entity.getComponent<TransformComponent>();
	renderData.emplaceTransform(transform.position, transform.rotation, transform.scale);
}

void Batcher::batchBV(const Entity& entity, RenderData& renderData) {
	const auto& bv = entity.getComponent<BoundingVolumeComponent>().bv;
	renderData.emplaceBV(bv->center(), bv->extents());
}

void Batcher::batchDebugMode(const Entity& entity, RenderData& renderData) {
	renderData.emplaceDebugMode(entity.getComponent<DebugComponent>().mode);
}

std::vector<uint32_t> Batcher::batchMeshes(RenderData& renderData, const std::vector<Mesh>& meshes) {
	std::vector<uint32_t> meshIndices;

	for (const auto& mesh: meshes) {
		meshIndices.push_back(buildState.meshIndex++);
		renderData.emplaceMesh(
			mesh.vao().id(),
			mesh.min(),
			mesh.max(),
			mesh.vertices().size(),
			mesh.indices().size());
	}

	return meshIndices;
}

MaterialBatch Batcher::batchMaterial(const uint32_t matID,
                                     const Entity& entity,
                                     RenderData& renderData) {
	const auto& matComponent = entity.getComponent<MaterialComponent>();
	auto& material = matComponent.materials->at(matID);
	material.idx = buildState.materialIndex++;

	renderData.emplaceHeightScale(matComponent.heightScale);
	renderData.emplaceMaterial(material.flags, material.color, material.alphaCutoff);

	DescriptorSet descriptorSet;
	uint32_t binding = 0;
	for (const auto& matTexture: material.textures) {
		descriptorSet.write(binding++, *matTexture.texture);
	}
	renderData.emplaceDescriptorSet(descriptorSet);

	MaterialBatch matBatch{
		.materialIndex = material.idx,
		.materialFlags = material.flags,
		.renderFlag = matComponent.renderFlag
	};

	return matBatch;
}

void Batcher::enqueueRenderGroup(const Entity& entity, QueueRegistry& queueRegistry, const MaterialBatch& matBatch) {
	if (matBatch.renderFlag == RenderFlag::InstancedPass) {
		const auto& instComponent = entity.getComponent<InstanceComponent>();
		RenderInstanceGroup group{entity.id(), matBatch, instComponent.transforms};

		for (const auto& rule: rules) {
			if ((matBatch.materialFlags & rule.flags) != MaterialFlag::None) {
				queueRegistry.emplace(rule.instancedQueue, group);
			}
		}
	} else {
		RenderGroup group{.entityID = entity.id(), .matBatch = matBatch};

		if (entity.hasComponent<DebugComponent>()) {
			queueRegistry.emplace("debug", group);
		}

		if (matBatch.renderFlag == RenderFlag::SkyboxPass) {
			queueRegistry.emplace("skybox", group);
		} else if (matBatch.renderFlag == RenderFlag::TerrainPass) {
			queueRegistry.emplace("terrain", group);
		}

		for (const auto& rule: rules) {
			if ((matBatch.materialFlags & rule.flags) != MaterialFlag::None) {
				queueRegistry.emplace(rule.queue, group);
			}
		}
	}
}
