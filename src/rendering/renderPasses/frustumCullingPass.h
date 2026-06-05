#pragma once
#include "IRenderPass.hpp"

class FrustumCullingPass final : public IRenderPass {
public:
	~FrustumCullingPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;
};