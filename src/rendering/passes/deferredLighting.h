#pragma once
#include <memory>
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"

namespace Model {
class Quad;
}

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IPass {
public:
	DeferredLightingPass();

	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsPipeline mPipeline{};
	GraphicsEncoder mEncoder{};
	std::unique_ptr<Model::Quad> mQuad;
};
