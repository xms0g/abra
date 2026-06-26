#pragma once
#include <array>
#include "IRenderPass.hpp"

class Shader;

class DebugPass final : public IRenderPass {
public:
	~DebugPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::array<const Shader*, 3> mDebugShaders{};
};
