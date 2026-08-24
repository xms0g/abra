#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "../descriptorSet.hpp"
#include "../material/material.hpp"
#include "../../math/matrix.hpp"

class FrameBuffer;

struct PBRBuffers {
	std::unique_ptr<FrameBuffer> environment;
	std::unique_ptr<FrameBuffer> irradiance;
	std::unique_ptr<FrameBuffer> prefilter;
	std::unique_ptr<FrameBuffer> brdfLUT;
};

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
		std::vector<MaterialFlag> flags;
		std::vector<DescriptorSet> descriptorSets;
		std::vector<glm::vec3> colors;
	} material;

	struct {
		std::vector<uint32_t> vaos;
		std::vector<glm::vec3> minCounts;
		std::vector<glm::vec3> maxCounts;
		std::vector<size_t> vertexCounts;
		std::vector<size_t> indexCounts;
	} mesh;

	void emplaceTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
		entity.positions.push_back(position);
		entity.rotations.push_back(rotation);
		entity.scales.push_back(scale);

		entity.models.emplace_back(math::modelMatrix(position, rotation, scale));
		entity.normals.emplace_back(math::normalMatrix(entity.models.back()));
	}

	void emplaceBV(const glm::vec3& center, const glm::vec3& extent) {
		entity.centers.emplace_back(center);
		entity.extents.emplace_back(extent);
	}

	void emplaceMesh(const uint32_t vaoid, const glm::vec3& minCount, const glm::vec3& maxCount, const size_t vertexCount, const size_t indexCount) {
		mesh.vaos.push_back(vaoid);
		mesh.minCounts.push_back(minCount);
		mesh.maxCounts.push_back(maxCount);
		mesh.vertexCounts.push_back(vertexCount);
		mesh.indexCounts.push_back(indexCount);
	}

	void emplaceMaterial(const MaterialFlag flags, const glm::vec3& color, const float alphaCutoff) {
		material.flags.push_back(flags);
		material.colors.push_back(color);
		material.alphaCutoffs.push_back(alphaCutoff);
	}

	void emplaceDescriptorSet(const DescriptorSet& descriptorSet) {
		material.descriptorSets.push_back(descriptorSet);
	}

	void emplaceHeightScale(const float heightScale) {
		entity.heightScales.push_back(heightScale);
	}

	void emplaceDebugMode(const uint32_t debugMode) {
		entity.debugModes.push_back(debugMode);
	}

	void updateTransform(const uint32_t entityID, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
		entity.positions[entityID] = position;
		entity.rotations[entityID] = rotation;
		entity.scales[entityID] = scale;

		const auto model = math::modelMatrix(position, rotation, scale);

		entity.models[entityID] = model;
		entity.normals[entityID] = math::normalMatrix(model);
	}

	void updateDebugMode(const uint32_t entityID, const uint32_t debugMode) {
		entity.debugModes[entityID] = debugMode;
	}
};
