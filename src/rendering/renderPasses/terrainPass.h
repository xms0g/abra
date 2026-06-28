#pragma once
#include <vector>
#include "IRenderPass.hpp"

struct RenderGroup;

class TerrainPass final: public IRenderPass {
public:
	~TerrainPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::vector<RenderGroup>* mObjects{nullptr};
};
