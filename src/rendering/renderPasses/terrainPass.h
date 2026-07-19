#pragma once
#include <vector>
#include "IRenderPass.hpp"

class Shader;
struct RenderGroup;

class TerrainPass final: public IRenderPass {
public:
	~TerrainPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::vector<RenderGroup>* mObjects{nullptr};
};
