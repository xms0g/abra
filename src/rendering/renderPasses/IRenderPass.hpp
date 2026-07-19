#pragma once

class RenderGraph;
class EventBus;
struct RenderContext;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void execute(const RenderContext& ctx, RenderGraph& graph) = 0;
};
