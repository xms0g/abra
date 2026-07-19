#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../../math/matrix.h"

struct RenderData {
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
		std::vector<uint32_t> textureTargets;
		std::vector<uint32_t> textures;
		std::vector<glm::vec3> colors;
	} material;

	struct {
		std::vector<uint32_t> vaos;
		std::vector<glm::vec3> minCounts;
		std::vector<glm::vec3> maxCounts;
		std::vector<size_t> vertexCounts;
		std::vector<size_t> indexCounts;
	} mesh;

	void updateTransform(const uint32_t entityID, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
		entity.positions[entityID] = position;
		entity.rotations[entityID] = rotation;
		entity.scales[entityID] = scale;

		const auto model = math::modelMatrix(position, rotation, scale);

		entity.models[entityID] = model;
		entity.normals[entityID] = math::normalMatrix(model);
	}
};
