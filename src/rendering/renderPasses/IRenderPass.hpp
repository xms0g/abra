#pragma once

class RenderGraph;
class EventBus;
struct RenderContext;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void configure(RenderContext& ctx, EventBus& eventBus) = 0;

	virtual void execute(const RenderContext& ctx, RenderGraph& graph) = 0;
};
