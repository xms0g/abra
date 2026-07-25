#pragma once
#include "IPass.hpp"
#include "../graphicsEncoder.h"

class ResolvePass final : public IPass {
public:
	ResolvePass();

	~ResolvePass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsEncoder mEncoder{};
};