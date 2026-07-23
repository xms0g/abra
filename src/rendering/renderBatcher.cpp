#include "renderBatcher.h"
#include "renderContext/renderData.hpp"
#include "renderContext/renderGroup.hpp"
#include "renderContext/renderFlags.hpp"
#include "renderContext/renderQueue.hpp"
#include "mesh/mesh.h"
#include "mesh/vertex.hpp"
#include "mesh/vertexArray.h"
#include "../math/boundingVolume.h"
#include "../math/matrix.h"
#include "../ECS/registry.h"
#include "../ECS/components/bv.hpp"
#include "../ECS/components/debug.hpp"
#include "../ECS/components/transform.hpp"
#include "../ECS/components/material.hpp"
#include "../ECS/components/mesh.hpp"
#include "../ECS/components/instance.hpp"

void RenderBatcher::build(RenderData& renderData, RenderQueue& renderQueue, const std::vector<Entity>& entities) {
	for (const auto& entity: entities) {
		batch(entity, renderData, renderQueue);
	}
}

void RenderBatcher::batch(const Entity& entity, RenderData& renderData, RenderQueue& renderQueue) {
	batchTransform(entity, renderData);
	batchBV(entity, renderData);
	batchDebugMode(entity, renderData);

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		const std::vector<uint32_t> meshIndices = batchMeshes(renderData, meshes);
		const MaterialBatch matBatch = batchMaterial(matID, entity, renderData, meshIndices);
		enqueueRenderGroup(entity, renderQueue, matBatch);
	}
}

void RenderBatcher::batchTransform(const Entity& entity, RenderData& renderData) {
	const auto& transform = entity.getComponent<TransformComponent>();
	renderData.emplaceTransform(transform.position, transform.rotation, transform.scale);
}

void RenderBatcher::batchBV(const Entity& entity, RenderData& renderData) {
	const auto& bv = entity.getComponent<BoundingVolumeComponent>().bv;
	renderData.emplaceBV(bv->center(), bv->extents());
}

void RenderBatcher::batchDebugMode(const Entity& entity, RenderData& renderData) {
	renderData.emplaceDebugMode(entity.getComponent<DebugComponent>().mode);
}

std::vector<uint32_t> RenderBatcher::batchMeshes(RenderData& renderData, const std::vector<Mesh>& meshes) {
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

MaterialBatch RenderBatcher::batchMaterial(
	const uint32_t matID,
	const Entity& entity,
	RenderData& renderData,
	const std::vector<uint32_t>& meshIndices) {
	const auto& matComponent = entity.getComponent<MaterialComponent>();
	auto& material = matComponent.materials->at(matID);
	material.idx = buildState.materialIndex++;

	renderData.emplaceHeightScale(matComponent.heightScale);
	renderData.emplaceMaterial(material.flags, material.textureTarget, material.color, material.alphaCutoff);

	for (const auto& texture: material.textures) {
		renderData.emplaceTexture(texture.id);
	}

	const size_t textureCount = material.textures.size();
	MaterialBatch matBatch{
		.materialIndex = material.idx,
		.materialFlags = material.flags,
		.renderFlag = matComponent.renderFlag,
		.textureOffset = buildState.textureOffset,
		.textureCount = textureCount,
		.shader = material.shader,
		.meshIndices = meshIndices
	};
	buildState.textureOffset += textureCount;

	return matBatch;
}

void RenderBatcher::enqueueRenderGroup(const Entity& entity, RenderQueue& renderQueue, const MaterialBatch& matBatch) {
	if (matBatch.renderFlag == INSTANCED_PASS) {
		const auto& instComponent = entity.getComponent<InstanceComponent>();
		RenderInstanceGroup group{entity.id(), matBatch, *instComponent.transforms};

		for (const auto& rule: rules) {
			if (matBatch.materialFlags & rule.flags) {
				renderQueue.emplace(rule.instancedQueue, group);
			}
		}
	} else {
		RenderGroup group{.entityID = entity.id(), .matBatch = matBatch};

		if (entity.hasComponent<DebugComponent>()) {
			renderQueue.emplace("debug", group);
		}

		if (matBatch.renderFlag == SKYBOX_PASS) {
			renderQueue.emplace("skybox", group);
		} else if (matBatch.renderFlag == TERRAIN_PASS) {
			renderQueue.emplace("terrain", group);
		}

		for (const auto& rule: rules) {
			if (matBatch.materialFlags & rule.flags) {
				renderQueue.emplace(rule.queue, group);
			}
		}
	}
}
