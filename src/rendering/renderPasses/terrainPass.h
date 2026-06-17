#pragma once
#include "IRenderPass.hpp"

class TerrainPass final: public IRenderPass {
public:
	~TerrainPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;
};
