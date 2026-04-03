#pragma once
#include <cstdint>
#include <vector>
#include "glad/glad.h"
#include "../math/matrix.h"

struct InstanceVBO {
	uint32_t buffer{};
	uint32_t offset{0};
};

struct InstanceData {
	glm::mat4 model;
	glm::mat3 normalMatrix;
	float padding[3];
};

namespace InstanceBufferBuilder {
template<typename T>
void prepareInstanceBuffer(const T& groups, InstanceVBO& vbo) {
	glGenBuffers(1, &vbo.buffer);

	size_t requiredGPUBufferSize = 0;
	for (const auto& [entity, transforms, matb]: groups) {
		for (const auto& mesh: *matb.meshes) {
			mesh.enableInstanceAttributes(vbo.buffer, vbo.offset);
		}

		const size_t count = transforms->size() / 9;
		const size_t instanceSize = count * sizeof(InstanceData);
		requiredGPUBufferSize += instanceSize;
		vbo.offset += static_cast<int>(instanceSize);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo.buffer);
	glBufferData(GL_ARRAY_BUFFER, static_cast<long>(requiredGPUBufferSize), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vbo.offset = 0;
}

template<typename T>
void uploadInstanceData(const T& groups, InstanceVBO& vbo) {
	for (const auto& [entity, transforms, matb]: groups) {
		std::vector<InstanceData> gpuData;
		gpuData.reserve(transforms->size() / 9);

		auto transform = *transforms;
		for (uint32_t i = 0; i < transform.size(); i += 9) {
			glm::vec3 pos{transform[i], transform[i + 1], transform[i + 2]};
			glm::vec3 rot{transform[i + 3], transform[i + 4], transform[i + 5]};
			glm::vec3 scale{transform[i + 6], transform[i + 7], transform[i + 8]};

			const glm::mat4 model = math::modelMatrix(pos, rot, scale);
			const glm::mat3 normal = math::normalMatrix(model);
			gpuData.emplace_back(model, normal);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo.buffer);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			vbo.offset,
			static_cast<long>(gpuData.size() * sizeof(InstanceData)),
			gpuData.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
}
