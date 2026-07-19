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
	explicit DeferredLightingPass(const RenderGraph& graph);

	~DeferredLightingPass() override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
