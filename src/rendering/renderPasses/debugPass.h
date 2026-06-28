#pragma once
#include <array>
#include <vector>
#include "IRenderPass.hpp"

struct RenderableObject;
class Shader;

class DebugPass final : public IRenderPass {
public:
	~DebugPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::array<const Shader*, 3> mDebugShaders{};
	std::vector<RenderableObject>* mObjects{nullptr};
};
