#pragma once
#include <vector>

struct InstanceGroup;
struct RenderGroup;
struct RenderCommand;
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
	// Render commands
	std::vector<RenderCommand> deferredCommands;
	std::vector<RenderCommand> forwardCommands;
	std::vector<RenderCommand> blendCommands;
	std::vector<RenderCommand> dbgCommands;
	std::vector<RenderCommand> shadowCommands;
	const Entity* skyboxEntity{};
};