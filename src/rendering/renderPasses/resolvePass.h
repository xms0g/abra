#pragma once
#include "baseRenderPass.hpp"

class ResolvePass final : public BaseRenderPass {
public:
	ResolvePass();

	~ResolvePass() override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;
};