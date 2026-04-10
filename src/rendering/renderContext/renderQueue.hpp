#pragma once
#include <vector>

struct InstanceGroup;
struct RenderGroup;
struct RenderableObject;
class Entity;

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
	std::vector<RenderableObject> shadowingObjects;
};