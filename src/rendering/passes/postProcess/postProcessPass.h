#pragma once
#include <memory>
#include <vector>
#include "basePostEffect.hpp"
#include "../IPass.hpp"

struct GuiPostProcessEvent;
class EventBus;
struct RenderContext;

namespace Model {
class Quad;
}

class Shader;

class PostProcessPass final : public IPass {
public:
	PostProcessPass();

	~PostProcessPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	void onGuiUpdate(const GuiPostProcessEvent& event);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::unique_ptr<Model::Quad> mQuad;
	std::vector<std::shared_ptr<BasePostEffect> > mEffects;
};
