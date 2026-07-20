#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include "baseRenderPass.hpp"

class VertexBuffer;
struct RenderInstanceGroup;

class InstancedPass final : public BaseRenderPass {
public:
	InstancedPass(const RenderContext& ctx, const RenderGraph& graph);

	~InstancedPass() override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	static void prepareInstanceBuffer(
		const std::vector<RenderInstanceGroup>& groups,
		const std::vector<uint32_t>& vaos,
		std::unique_ptr<VertexBuffer>& vbo);

	static void uploadInstanceData(const std::vector<RenderInstanceGroup>& groups, const VertexBuffer& vbo);

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;

	std::vector<RenderInstanceGroup>* mOpaqueObjects{nullptr};
	std::vector<RenderInstanceGroup>* mTransparentObjects{nullptr};
};
