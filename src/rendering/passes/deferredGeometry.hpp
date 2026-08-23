#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.hpp"
#include "../context/renderQueue.hpp"
#include "../pushConstants/materialPushConstants.hpp"
#include "../pushConstants/transformPushConstants.hpp"

struct DrawCommand;

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

	struct PushConstants {
		MaterialPushConstants material;
		TransformPushConstants transform;
	};

	ResourceIndexes mIndexes{};
	GraphicsPipeline mPipeline{};
	RenderQueue<DrawCommand>* mCommands{nullptr};
};
