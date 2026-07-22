#pragma once
#include <array>
#include <vector>
#include "baseRenderPass.hpp"

struct RenderableObject;
class Shader;

class DebugPass final : public BaseRenderPass {
public:
	DebugPass();

	~DebugPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	std::array<const Shader*, 3> mDebugShaders{};
	std::vector<RenderableObject>* mObjects{nullptr};
};
