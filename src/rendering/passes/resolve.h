#pragma once
#include <cstdint>
#include "IPass.hpp"

class ResolvePass final : public IPass {
public:
	ResolvePass();

	~ResolvePass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	struct ResourceIndexes {
		uint32_t sceneBuffer;
		uint32_t intermediateBuffer;
	};

	ResourceIndexes mIndexes{};
};
