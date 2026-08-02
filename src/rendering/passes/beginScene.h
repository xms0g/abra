#pragma once
#include "IPass.hpp"

class BeginScenePass final : public IPass {
public:
	BeginScenePass();

	~BeginScenePass() override;

	void configure(
		const RenderContext& ctx,
		const FrameGraph& graph,
		GraphicsEncoder& encoder,
		EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;
};
