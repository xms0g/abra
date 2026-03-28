#pragma once
#include <vector>

struct InstanceGroup;
struct RenderGroup;
struct RenderableObject;
class Entity;

struct RenderQueue {
	// Instance groups
	std::vector<InstanceGroup> opaqueInstancedGroups;
	std::vector<InstanceGroup> cutoutInstancedGroups;
	std::vector<InstanceGroup> blendInstancedGroups;
	// Render groups
	std::vector<RenderGroup> debugGroups;
	std::vector<RenderGroup> forwardOpaqueGroups;
	std::vector<RenderGroup> deferredGroups;
	std::vector<RenderGroup> blendGroups;
	std::vector<RenderGroup> shadowGroups;
	std::vector<RenderGroup> pbrGroups;
	std::vector<RenderGroup> skybox;
	// Renderable Objects
	std::vector<RenderableObject> deferredObjects;
	std::vector<RenderableObject> forwardObjects;
	std::vector<RenderableObject> blendObjects;
	std::vector<RenderableObject> dbgObjects;
	std::vector<RenderableObject> shadowingObjects;
	std::vector<RenderableObject> pbrObjects;
};