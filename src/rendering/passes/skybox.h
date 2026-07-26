#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../context/renderQueue.hpp"

struct DrawCommand;

class SkyboxPass final : public IPass {
public:
	SkyboxPass();

	~SkyboxPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsPipeline mPipeline{};
	GraphicsEncoder mEncoder{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
