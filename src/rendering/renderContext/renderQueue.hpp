#pragma once
#include <vector>
#include "glm/glm.hpp"

struct InstanceGroup;
struct RenderGroup;
struct RenderableObject;

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

	struct {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> rotations;
		std::vector<glm::vec3> scales;
		std::vector<glm::mat4> models;
		std::vector<glm::mat3> normals;
		std::vector<glm::vec3> centers;
		std::vector<glm::vec3> extents;
		std::vector<uint32_t> debugModes;
		std::vector<float> heightScales;
	} entity;

	struct {
		std::vector<float> alphaCutoffs;
		std::vector<uint32_t> flags;
		std::vector<std::vector<uint32_t>> textures;
		std::vector<glm::vec3> colors;
	} material;

	struct {
		std::vector<uint32_t> vaos;
		std::vector<glm::vec3> minCounts;
		std::vector<glm::vec3> maxCounts;
		std::vector<size_t> vertexCounts;
		std::vector<size_t> indexCounts;
	} mesh;
};
