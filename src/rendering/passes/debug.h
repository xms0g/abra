#pragma once
#include <array>
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../context/renderQueue.hpp"

struct DrawCommand;
struct VisibleObject;
class Shader;

class DebugPass final : public IPass {
public:
	DebugPass();

	~DebugPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	std::array<GraphicsPipeline, 3> mPipelines{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
