#pragma once
#include "IPass.hpp"
#include "../context/renderQueue.hpp"

struct VisibleObject;

class ForwardPass final : public IPass {
public:
	ForwardPass();

	~ForwardPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	RenderQueue<VisibleObject>* mOpaqueObjects{nullptr};
	RenderQueue<VisibleObject>* mTransparentObjects{nullptr};
};
