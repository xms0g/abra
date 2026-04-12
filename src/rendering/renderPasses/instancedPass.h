#pragma once
#include <memory>
#include <vector>
#include "IRenderPass.hpp"
#include "../buffers/vertexBuffer.h"
#include "../../math/matrix.h"

class InstancedPass final : public IRenderPass {
public:
	~InstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	template<typename T>
	void prepareInstanceBuffer(const T& groups, std::unique_ptr<VertexBuffer>& vbo);

	template<typename T>
	void uploadInstanceData(const T& groups, const VertexBuffer& vbo);

	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;
};

template<typename T>
void InstancedPass::prepareInstanceBuffer(const T& groups, std::unique_ptr<VertexBuffer>& vbo) {
	vbo = std::make_unique<VertexBuffer>(DYNAMIC);

	size_t totalRequiredSize = 0;
	for (const auto& [entity, transforms, matb] : groups) {
		const size_t instanceCount = transforms->size() / 9;
		totalRequiredSize += instanceCount * sizeof(InstanceData);
	}

	// Allocate the full block of memory once
	vbo->bind();
	vbo->setData(nullptr, static_cast<uint32_t>(totalRequiredSize), 0);

	// Setup attributes now that the buffer is allocated
	uint32_t currentOffset = 0;
	for (const auto& [entity, transforms, matb] : groups) {
		for (auto& mesh : *matb.meshes) {
			mesh.bind();
			mesh.enableInstanceAttributes(
				*vbo,
				sizeof(InstanceData),
				currentOffset,
				offsetof(InstanceData, modelMatrix),
				offsetof(InstanceData, normalMatrix));
		}
		const size_t instanceCount = transforms->size() / 9;
		currentOffset += static_cast<uint32_t>(instanceCount * sizeof(InstanceData));
	}

	vbo->unbind();
}

template<typename T>
void InstancedPass::uploadInstanceData(const T& groups, const VertexBuffer& vbo) {
	vbo.bind();

	uint32_t currentOffset = 0;
	for (const auto& [entity, transforms, matb]: groups) {
		std::vector<InstanceData> gpuData;
		const size_t instanceCount = transforms->size() / 9;
		gpuData.reserve(instanceCount);

		const auto& t = *transforms;
		for (size_t i = 0; i < t.size(); i += 9) {
			glm::vec3 pos{t[i], t[i + 1], t[i + 2]};
			glm::vec3 rot{t[i + 3], t[i + 4], t[i + 5]};
			glm::vec3 scale{t[i + 6], t[i + 7], t[i + 8]};

			glm::mat4 model = math::modelMatrix(pos, rot, scale);
			gpuData.emplace_back(model, math::normalMatrix(model));
		}

		const size_t uploadSize = gpuData.size() * sizeof(InstanceData);
		vbo.setData(gpuData.data(), static_cast<uint32_t>(uploadSize), currentOffset);
		currentOffset += static_cast<uint32_t>(uploadSize);
	}

	vbo.unbind();
}
