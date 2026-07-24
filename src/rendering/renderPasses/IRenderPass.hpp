#pragma once

class RenderGraph;
class EventBus;
struct RenderContext;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) = 0;

	virtual void execute(const RenderContext& ctx, const RenderGraph& graph) = 0;
};
