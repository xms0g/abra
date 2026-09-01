#pragma once
#include <memory>
#include <span>
#include "IPass.hpp"
#include "../graphicsPipeline.hpp"

class VertexBuffer;
struct RenderInstanceGroup;

class InstancedOpaquePass final : public IPass {
public:
	InstancedOpaquePass();

	~InstancedOpaquePass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	void prepareInstanceBuffer(const RenderContext& ctx);

	void uploadInstanceData() const;

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	std::unique_ptr<VertexBuffer> mVBO;
	std::span<RenderInstanceGroup> mObjects;
};
