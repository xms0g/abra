#pragma once
#include "IRenderPass.hpp"

class ResolvePass final : public IRenderPass {
public:
	~ResolvePass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};