#pragma once

class FrameGraph;
class EventBus;
struct RenderContext;

class IPass {
public:
	virtual ~IPass() = default;

	virtual void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) = 0;

	virtual void execute(const RenderContext& ctx, const FrameGraph& graph) = 0;
};
