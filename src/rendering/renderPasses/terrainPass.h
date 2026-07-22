#pragma once
#include <vector>
#include "baseRenderPass.hpp"

class Shader;
struct RenderGroup;

class TerrainPass final: public BaseRenderPass {
public:
	TerrainPass();

	~TerrainPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::vector<RenderGroup>* mObjects{nullptr};
};
