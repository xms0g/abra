#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../context/renderQueue.hpp"

class Shader;
struct RenderGroup;

class TerrainPass final: public IPass {
public:
	TerrainPass();

	~TerrainPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	GraphicsPipeline mPipeline{};
	GraphicsEncoder mEncoder{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
