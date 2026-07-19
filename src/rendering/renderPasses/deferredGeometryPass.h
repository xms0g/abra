#pragma once
#include <memory>
#include <vector>
#include "IRenderPass.hpp"

struct RenderableObject;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IRenderPass {
public:
	~DeferredGeometryPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	std::unique_ptr<FrameBuffer> mGBuffer;
	const Shader* mShader{nullptr};
	std::vector<RenderableObject>* mObjects{nullptr};
};
