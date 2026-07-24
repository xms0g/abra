#pragma once
#include "IRenderPass.hpp"

class ResolvePass final : public IRenderPass {
public:
	ResolvePass();

	~ResolvePass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;
};