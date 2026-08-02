#pragma once
#include "IPass.hpp"
#include <span>
#include "../context/renderQueue.hpp"

struct DrawCommand;

namespace math {
struct Frustum;
}

struct RenderGroup;

class CullingPass final : public IPass {
public:
	CullingPass();

	~CullingPass() override;

	void configure(
		const RenderContext& ctx,
		const FrameGraph& graph,
		GraphicsEncoder& encoder,
		EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	static void cullScene(
		const RenderContext& ctx,
		const math::Frustum& frustum,
		std::span<RenderGroup> groups,
		RenderQueue<DrawCommand>& outQueue);

	std::span<RenderGroup> mOpaqueGroups{};
	std::span<RenderGroup> mUnlitGroups{};
	std::span<RenderGroup> mBlendGroups{};
	std::span<RenderGroup> mDeferredGroups{};
	std::span<RenderGroup> mDebugGroups{};
	std::span<RenderGroup> mTerrainGroups{};
	std::span<RenderGroup> mSkyboxGroups{};
	RenderQueue<DrawCommand>* mOpaqueCommands{nullptr};
	RenderQueue<DrawCommand>* mUnlitCommands{nullptr};
	RenderQueue<DrawCommand>* mBlendCommands{nullptr};
	RenderQueue<DrawCommand>* mDeferredCommands{nullptr};
	RenderQueue<DrawCommand>* mDebugCommands{nullptr};
	RenderQueue<DrawCommand>* mTerrainCommands{nullptr};
	RenderQueue<DrawCommand>* mSkyboxCommands{nullptr};
};
