#pragma once
#include "IRenderPass.hpp"

class ResolvePass final : public IRenderPass {
public:
	~ResolvePass() override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;
};