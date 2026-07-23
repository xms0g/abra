#pragma once
#include "baseRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

namespace math {
struct Frustum;
}

struct RenderGroup;
struct RenderableObject;

class CullingPass final : public BaseRenderPass {
public:
	CullingPass();

	~CullingPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	static void cullScene(
		const RenderContext& ctx,
		const math::Frustum& frustum,
		const RenderQueue<RenderGroup>& groups,
		RenderQueue<RenderableObject>& outQueue);

	RenderQueue<RenderGroup>* mOpaqueGroups{nullptr};
	RenderQueue<RenderGroup>* mBlendGroups{nullptr};
	RenderQueue<RenderGroup>* mDeferredGroups{nullptr};
	RenderQueue<RenderGroup>* mDebugGroups{nullptr};
	RenderQueue<RenderableObject>* mVisibleOpaque{nullptr};
	RenderQueue<RenderableObject>* mVisibleBlend{nullptr};
	RenderQueue<RenderableObject>* mVisibleDeferred{nullptr};
	RenderQueue<RenderableObject>* mVisibleDebug{nullptr};
};
