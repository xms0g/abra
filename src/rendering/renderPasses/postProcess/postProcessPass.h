#pragma once
#include <memory>
#include <vector>
#include "basePostEffect.hpp"
#include "../IRenderPass.hpp"

struct GuiPostProcessEvent;
class EventBus;
struct RenderContext;

namespace Models {
class Quad;
}

class Shader;

class PostProcessPass final : public IRenderPass {
public:
	~PostProcessPass() override;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void onGuiUpdate(const GuiPostProcessEvent& event);

	PingPongBuffer mPingPong;
	std::unique_ptr<Models::Quad> mQuad;
	std::vector<std::unique_ptr<BasePostEffect> > mEffects;
};
