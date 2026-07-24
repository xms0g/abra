#pragma once
#include <memory>
#include "IPass.hpp"

namespace Model {
class SingleQuad;
}

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public IPass {
public:
	DeferredLightingPass();

	~DeferredLightingPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
