#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"

class CubemapBuffer;
class FrameBuffer;
class Shader;

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
	GraphicsPipeline mPipeline{};
};
