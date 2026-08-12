#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../context/renderQueue.hpp"

struct DrawCommand;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IPass {
public:
	DeferredGeometryPass();

	~DeferredGeometryPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t gBuffer;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
