#pragma once
#include <vector>
#include "baseRenderPass.hpp"

struct RenderableObject;

class ForwardPass final : public BaseRenderPass {
public:
	ForwardPass();

	~ForwardPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	std::vector<RenderableObject>* mOpaqueObjects{nullptr};
	std::vector<RenderableObject>* mTransparentObjects{nullptr};
};
