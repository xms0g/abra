#pragma once
#include <array>
#include "IPass.hpp"
#include "../context/renderQueue.hpp"

struct VisibleObject;
class Shader;

class DebugPass final : public IPass {
public:
	DebugPass();

	~DebugPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	std::array<const Shader*, 3> mDebugShaders{};
	RenderQueue<VisibleObject>* mObjects{nullptr};
};
