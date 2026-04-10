#pragma once
#include "IRenderPass.hpp"

class ForwardPass final : public IRenderPass {
public:
	~ForwardPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};
