#pragma once
#include <vector>
#include "glm/glm.hpp"

struct InstanceGroup;
struct RenderGroup;
struct RenderableObject;
class Entity;

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 model;
	glm::mat3 normal;
};

struct BV {
	glm::vec3 center;
	glm::vec3 extents;
};

struct RenderQueue {
	// Instance groups
	std::vector<InstanceGroup> opaqueInstancedGroups;
	std::vector<InstanceGroup> blendInstancedGroups;
	// Render groups
	std::vector<RenderGroup> debugGroups;
	std::vector<RenderGroup> opaqueGroups;
	std::vector<RenderGroup> deferredGroups;
	std::vector<RenderGroup> blendGroups;
	std::vector<RenderGroup> shadowGroups;
	std::vector<RenderGroup> skybox;
	// Renderable Objects
	std::vector<RenderableObject> deferredObjects;
	std::vector<RenderableObject> opaqueObjects;
	std::vector<RenderableObject> blendObjects;
	std::vector<RenderableObject> dbgObjects;
	std::vector<RenderableObject> shadowedObjects;

	std::vector<Transform> entityTransforms;
	std::vector<BV> entityBVs;
	std::vector<uint32_t> entityDebugModes;
	std::vector<float> entityHeightScales;
	std::vector<float> matAlphaCutoffs;
	std::vector<uint32_t> matFlags;
	std::vector<std::vector<uint32_t>> matTextures;
};
