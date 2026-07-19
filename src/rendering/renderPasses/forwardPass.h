#pragma once
#include <vector>
#include "IRenderPass.hpp"

struct RenderableObject;

class ForwardPass final : public IRenderPass {
public:
	ForwardPass(const RenderContext& ctx, const RenderGraph& graph);

	~ForwardPass() override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	std::vector<RenderableObject>* mOpaqueObjects{nullptr};
	std::vector<RenderableObject>* mTransparentObjects{nullptr};
};
