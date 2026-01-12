#pragma once
#include "IRenderPass.hpp"

class PBRPass final : public IRenderPass {
public:
	~PBRPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};
