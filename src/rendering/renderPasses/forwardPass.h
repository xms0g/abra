#pragma once
#include "IRenderPass.hpp"

class ForwardPass final : public IRenderPass {
public:
	~ForwardPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;
};
