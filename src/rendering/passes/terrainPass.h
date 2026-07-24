#pragma once
#include "IPass.hpp"
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
	const Shader* mShader{nullptr};
	RenderQueue<RenderGroup>* mObjects{nullptr};
};
