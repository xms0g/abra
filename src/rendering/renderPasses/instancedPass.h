#pragma once
#include <memory>
#include <vector>
#include "IRenderPass.hpp"

class VertexBuffer;
struct InstanceGroup;

class InstancedPass final : public IRenderPass {
public:
	InstancedPass();

	~InstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	void prepareInstanceBuffer(const std::vector<InstanceGroup>& groups, std::unique_ptr<VertexBuffer>& vbo);

	void uploadInstanceData(const std::vector<InstanceGroup>& groups, const VertexBuffer& vbo);

	std::unique_ptr<VertexBuffer> mOpaqueVBO;
	std::unique_ptr<VertexBuffer> mBlendVBO;
};
