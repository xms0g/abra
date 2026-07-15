#pragma once
#include <vector>
#include "material/material.hpp"

class Entity;
class RenderQueue;
struct RenderData;

class RenderBatcher {
public:
	void build(RenderData& renderData, RenderQueue& renderQueue, const std::vector<Entity>& entities);

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
	} rules[4] = {
		{CASTSHADOW, "shadow", "shadow"},
		{PBR, "deferred", "deferred"},
		{OPAQUE, "opaque", "opaqueInstanced"},
		{BLEND, "blend", "blendInstanced"},
	};

	void batch(const Entity& entity, RenderData& renderData, RenderQueue& renderQueue);
};
