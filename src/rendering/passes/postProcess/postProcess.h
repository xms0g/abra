#pragma once
#include <memory>
#include <vector>
#include "basePostEffect.hpp"
#include "../IPass.hpp"
#include "../../graphicsPipeline.h"
#include "../../descriptorSet.h"

struct GuiPostProcessEvent;
class EventBus;
struct RenderContext;

class Shader;

class PostProcessPass final : public IPass {
public:
	PostProcessPass();

	~PostProcessPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	void onGuiUpdate(const GuiPostProcessEvent& event);

	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::array<DescriptorSet, 2> mRenderTargetsDescSets{};
	std::vector<std::shared_ptr<BasePostEffect> > mEffects;
};
