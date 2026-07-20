#pragma once
#include <memory>
#include <vector>
#include "basePostEffect.hpp"
#include "../baseRenderPass.hpp"

struct GuiPostProcessEvent;
class EventBus;
struct RenderContext;

namespace Model {
class Quad;
}

class Shader;

class PostProcessPass final : public BaseRenderPass {
public:
	PostProcessPass(const RenderGraph& graph, EventBus& eventBus);

	~PostProcessPass() override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	void onGuiUpdate(const GuiPostProcessEvent& event);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::unique_ptr<Model::Quad> mQuad;
	std::vector<std::shared_ptr<BasePostEffect> > mEffects;
};
