#pragma once
#include "IPass.hpp"
#include "../context/renderQueue.hpp"

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
		RenderQueue<VisibleObject>& outQueue);

	RenderQueue<RenderGroup>* mOpaqueGroups{nullptr};
	RenderQueue<RenderGroup>* mBlendGroups{nullptr};
	RenderQueue<RenderGroup>* mDeferredGroups{nullptr};
	RenderQueue<RenderGroup>* mDebugGroups{nullptr};
	RenderQueue<VisibleObject>* mVisibleOpaque{nullptr};
	RenderQueue<VisibleObject>* mVisibleBlend{nullptr};
	RenderQueue<VisibleObject>* mVisibleDeferred{nullptr};
	RenderQueue<VisibleObject>* mVisibleDebug{nullptr};
};
