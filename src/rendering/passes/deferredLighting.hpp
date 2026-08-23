#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.hpp"

class DeferredLightingPass final : public IPass {
public:
	DeferredLightingPass();

	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t gBuffer;
		uint32_t sceneBuffer;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
};
