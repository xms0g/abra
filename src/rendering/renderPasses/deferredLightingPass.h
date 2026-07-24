#pragma once
#include <memory>
#include "IRenderPass.hpp"

namespace Model {
class SingleQuad;
}

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IRenderPass {
public:
	explicit DeferredLightingPass();

	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
