#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include "IRenderPass.hpp"

class VertexBuffer;
struct InstanceGroup;

class InstancedPass final : public IRenderPass {
public:
	InstancedPass();

	~InstancedPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	void prepareInstanceBuffer(
		const std::vector<InstanceGroup>& groups,
		const std::vector<uint32_t>& vaos,
		std::unique_ptr<VertexBuffer>& vbo);

	void uploadInstanceData(const std::vector<InstanceGroup>& groups, const VertexBuffer& vbo);

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;
};
