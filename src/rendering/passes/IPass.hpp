#pragma once

class GraphicsEncoder;
class FrameGraph;
class EventBus;
struct RenderContext;

class IPass {
public:
	virtual ~IPass() = default;

	virtual void configure(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder, EventBus& eventBus) = 0;

	virtual void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) = 0;
};
