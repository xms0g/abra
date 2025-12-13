#pragma once
#include "IRenderPass.hpp"

class BlendPass final : public IRenderPass {
public:
	~BlendPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};
