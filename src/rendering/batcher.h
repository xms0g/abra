#pragma once
#include <vector>
#include "material/material.hpp"

class Entity;
class QueueRegistry;
struct RenderData;

class Batcher {
public:
	void build(RenderData& renderData, QueueRegistry& queueRegistry, const std::vector<Entity>& entities);

private:
	struct BuildState {
		uint32_t materialIndex{0};
		uint32_t textureOffset{0};
		uint32_t meshIndex{0};
	} buildState{};

	struct PassRule {
		uint32_t flags;
		std::string queue;
		std::string instancedQueue;
	} rules[5] = {
		{.flags = CASTSHADOW, .queue = "shadow", .instancedQueue = ""},
		{.flags = PBR, .queue = "deferred", .instancedQueue = ""},
		{.flags = UNLIT, .queue = "unlit", .instancedQueue = ""},
		{.flags = OPAQUE, .queue = "opaque", .instancedQueue = "opaqueInstanced"},
		{.flags = BLEND, .queue = "blend", .instancedQueue = "blendInstanced"},
	};

	void batch(const Entity& entity, RenderData& renderData, QueueRegistry& queueRegistry);

	static void batchTransform(const Entity& entity, RenderData& renderData);

	static void batchBV(const Entity& entity, RenderData& renderData);

	static void batchDebugMode(const Entity& entity, RenderData& renderData);

	std::vector<uint32_t> batchMeshes(RenderData& renderData, const std::vector<Mesh>& meshes);

	MaterialBatch batchMaterial(
		uint32_t matID,
		const Entity& entity,
		RenderData& renderData,
		const std::vector<uint32_t>& meshIndices);

	void enqueueRenderGroup(const Entity& entity, QueueRegistry& queueRegistry, const MaterialBatch& matBatch);
};
