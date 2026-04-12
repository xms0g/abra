#pragma once
#include <memory>
#include <vector>
#include "IRenderPass.hpp"
#include "../buffers/vertexBuffer.h"
#include "../../math/matrix.h"

struct InstanceGroup;

class InstancedPass final : public IRenderPass {
public:
	~InstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	void prepareInstanceBuffer(const std::vector<InstanceGroup>& groups, std::unique_ptr<VertexBuffer>& vbo);

	void uploadInstanceData(const std::vector<InstanceGroup>& groups, const VertexBuffer& vbo);

	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;
};
