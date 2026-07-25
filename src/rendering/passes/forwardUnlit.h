#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../context/renderQueue.hpp"

struct DrawCommand;

class ForwardUnlitPass final : public IPass {
public:
	ForwardUnlitPass();

	~ForwardUnlitPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsPipeline mPipeline{};
	GraphicsEncoder mEncoder{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};

