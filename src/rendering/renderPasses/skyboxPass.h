#pragma once
#include <vector>

#include "IRenderPass.hpp"

struct RenderGroup;

class SkyboxPass final : public IRenderPass {
public:
	~SkyboxPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::vector<RenderGroup>* mObjects{nullptr};
};
