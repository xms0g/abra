#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include "IRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

class VertexBuffer;
struct RenderInstanceGroup;

class InstancedPass final : public IRenderPass {
public:
	InstancedPass();

	~InstancedPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	struct InstanceData {
		glm::mat4 modelMatrix;
		glm::mat3 normalMatrix;
		float padding[3];
	};

	static void prepareInstanceBuffer(
		const RenderQueue<RenderInstanceGroup>& groups,
		const std::vector<uint32_t>& vaos,
		std::unique_ptr<VertexBuffer>& vbo);

	static void uploadInstanceData(const RenderQueue<RenderInstanceGroup>& groups, const VertexBuffer& vbo);

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;

	RenderQueue<RenderInstanceGroup>* mOpaqueObjects{nullptr};
	RenderQueue<RenderInstanceGroup>* mTransparentObjects{nullptr};
};
