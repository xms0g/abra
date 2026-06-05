#pragma once
#include "IRenderPass.hpp"

class SkyboxPass final : public IRenderPass {
public:
	~SkyboxPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;
};