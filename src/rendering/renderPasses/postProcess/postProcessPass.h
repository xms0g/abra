#pragma once
#include <memory>
#include <vector>
#include "basePostEffect.hpp"
#include "../IRenderPass.hpp"

struct GuiPostProcessEvent;
class EventBus;
struct RenderContext;

namespace Model {
class Quad;
}

class Shader;

class PostProcessPass final : public IRenderPass {
public:
	~PostProcessPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void onGuiUpdate(const GuiPostProcessEvent& event);

	PingPongBuffer mRenderTargets;
	std::unique_ptr<Model::Quad> mQuad;
	std::vector<std::shared_ptr<BasePostEffect> > mEffects;
};
