#pragma once
#include <array>
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../context/renderQueue.hpp"

struct VisibleObject;
class Shader;

class DebugPass final : public IPass {
public:
	DebugPass();

	~DebugPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsEncoder mEncoder{};
	std::array<GraphicsPipeline, 3> mPipelines{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
