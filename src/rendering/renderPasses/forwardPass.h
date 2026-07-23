#pragma once
#include "baseRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

struct RenderableObject;

class ForwardPass final : public BaseRenderPass {
public:
	ForwardPass();

	~ForwardPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	RenderQueue<RenderableObject>* mOpaqueObjects{nullptr};
	RenderQueue<RenderableObject>* mTransparentObjects{nullptr};
};
