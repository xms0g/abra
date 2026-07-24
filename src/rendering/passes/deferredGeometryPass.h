#pragma once
#include "IPass.hpp"
#include "../context/renderQueue.hpp"

struct VisibleObject;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IPass {
public:
	DeferredGeometryPass();

	~DeferredGeometryPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	RenderQueue<VisibleObject>* mObjects{nullptr};
};
