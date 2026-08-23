#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.hpp"
#include "../context/renderQueue.hpp"

struct DrawCommand;

class SkyboxPass final : public IPass {
public:
	SkyboxPass();

	~SkyboxPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t sceneBuffer;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
