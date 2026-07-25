#pragma once
#include "IPass.hpp"
#include "../context/renderQueue.hpp"

struct DrawCommand;

namespace math {
struct Frustum;
}

struct RenderGroup;
struct VisibleObject;

class CullingPass final : public IPass {
public:
	CullingPass();

	~CullingPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	static void cullScene(
		const RenderContext& ctx,
		const math::Frustum& frustum,
		const RenderQueue<RenderGroup>& groups,
		RenderQueue<DrawCommand>& outQueue);

	RenderQueue<RenderGroup>* mOpaqueGroups{nullptr};
	RenderQueue<RenderGroup>* mUnlitGroups{nullptr};
	RenderQueue<RenderGroup>* mBlendGroups{nullptr};
	RenderQueue<RenderGroup>* mDeferredGroups{nullptr};
	RenderQueue<RenderGroup>* mDebugGroups{nullptr};
	RenderQueue<DrawCommand>* mOpaqueCommands{nullptr};
	RenderQueue<DrawCommand>* mUnlitCommands{nullptr};
	RenderQueue<DrawCommand>* mBlendCommands{nullptr};
	RenderQueue<DrawCommand>* mDeferredCommands{nullptr};
	RenderQueue<DrawCommand>* mDebugCommands{nullptr};
};
