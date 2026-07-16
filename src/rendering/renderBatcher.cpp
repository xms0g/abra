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

	for (auto& [matID, meshes]: *entity.getComponent<MeshComponent>().meshes) {
		const std::vector<uint32_t> meshIndices = batchMeshes(renderData, meshes);
		const MaterialBatch matBatch = batchMaterial(matID, entity, renderData, meshIndices);
		enqueueRenderGroup(entity, renderQueue, matBatch);
	}
}

void RenderBatcher::batchTransform(const Entity& entity, RenderData& renderData) {
	const auto& transform = entity.getComponent<TransformComponent>();
	const auto modelMat = math::modelMatrix(transform.position, transform.rotation, transform.scale);
	const auto normalMat = math::normalMatrix(modelMat);

	renderData.entity.positions.push_back(transform.position);
	renderData.entity.rotations.push_back(transform.rotation);
	renderData.entity.scales.push_back(transform.scale);
	renderData.entity.models.push_back(modelMat);
	renderData.entity.normals.push_back(normalMat);
}

void RenderBatcher::batchBV(const Entity& entity, RenderData& renderData) {
	const auto& bv = entity.getComponent<BoundingVolumeComponent>().bv;
	renderData.entity.centers.push_back(bv->center());
	renderData.entity.extents.push_back(bv->extents());

	renderData.entity.debugModes.emplace_back(0);
}

std::vector<uint32_t> RenderBatcher::batchMeshes(RenderData& renderData, const std::vector<Mesh>& meshes) {
	std::vector<uint32_t> meshIndices;

	for (const auto& mesh: meshes) {
		meshIndices.push_back(buildState.meshIndex++);
		renderData.mesh.vaos.push_back(mesh.vao().id());
		renderData.mesh.vertexCounts.push_back(mesh.vertices().size());
		renderData.mesh.indexCounts.push_back(mesh.indices().size());
		renderData.mesh.maxCounts.push_back(mesh.max());
		renderData.mesh.minCounts.push_back(mesh.min());
	}

	return meshIndices;
}

MaterialBatch RenderBatcher::batchMaterial(
	const uint32_t matID,
	const Entity& entity,
	RenderData& renderData,
	const std::vector<uint32_t>& meshIndices) {
	auto& matComponent = entity.getComponent<MaterialComponent>();
	renderData.entity.heightScales.emplace_back(matComponent.heightScale);

	auto& material = matComponent.materials->at(matID);
	material.idx = buildState.materialIndex++;

	renderData.material.flags.push_back(material.flags);
	renderData.material.textureTargets.push_back(material.textureTarget);
	renderData.material.alphaCutoffs.push_back(material.alphaCutoff);
	renderData.material.colors.push_back(material.color);

	for (const auto& texture: material.textures) {
		renderData.material.textures.push_back(texture.id);
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
		const RenderInstanceGroup group{entity.id(), matBatch, *instComponent.transforms};

		for (const auto& rule: rules) {
			if (matBatch.materialFlags & rule.flags) {
				renderQueue.get<std::vector<RenderInstanceGroup> >(rule.instancedQueue).push_back(group);
			}
		}
	} else {
		const RenderGroup group{entity.id(), matBatch};

		if (entity.hasComponent<DebugComponent>()) {
			renderQueue.get<std::vector<RenderGroup> >("debug").push_back(group);
		}

		if (matBatch.renderFlag == SKYBOX_PASS) {
			renderQueue.get<std::vector<RenderGroup> >("skybox").push_back(group);
		} else if (matBatch.renderFlag == TERRAIN_PASS) {
			renderQueue.get<std::vector<RenderGroup> >("terrain").push_back(group);
		}

		for (const auto& rule: rules) {
			if (matBatch.materialFlags & rule.flags) {
				renderQueue.get<std::vector<RenderGroup> >(rule.queue).push_back(group);
			}
		}
	}
}
