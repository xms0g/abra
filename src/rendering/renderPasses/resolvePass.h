#pragma once
#include "IRenderPass.hpp"

class ResolvePass final : public IRenderPass {
public:
	~ResolvePass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;
};