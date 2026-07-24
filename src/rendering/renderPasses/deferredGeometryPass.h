#pragma once
#include "IRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

struct RenderableObject;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IRenderPass {
public:
	DeferredGeometryPass();

	~DeferredGeometryPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	RenderQueue<RenderableObject>* mObjects{nullptr};
};
