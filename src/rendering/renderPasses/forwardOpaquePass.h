#pragma once
#include "IRenderPass.hpp"

class ForwardOpaquePass final : public IRenderPass {
public:
	~ForwardOpaquePass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};
