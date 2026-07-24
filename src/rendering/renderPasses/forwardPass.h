#pragma once
#include "IRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

struct RenderableObject;

class ForwardPass final : public IRenderPass {
public:
	~ForwardPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	RenderQueue<RenderableObject>* mOpaqueObjects{nullptr};
	RenderQueue<RenderableObject>* mTransparentObjects{nullptr};
};
