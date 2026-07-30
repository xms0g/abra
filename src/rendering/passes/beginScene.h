#pragma once
#include "IPass.hpp"
#include "../graphicsEncoder.h"

class BeginScenePass final : public IPass {
public:
	BeginScenePass();

	~BeginScenePass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsEncoder mEncoder{};
};