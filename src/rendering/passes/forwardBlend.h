#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../context/renderQueue.hpp"

struct DrawCommand;

class ForwardBlendPass final : public IPass {
public:
	ForwardBlendPass();

	~ForwardBlendPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	GraphicsPipeline mPipeline{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
