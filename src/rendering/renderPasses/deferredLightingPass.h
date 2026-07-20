#pragma once
#include <memory>
#include "baseRenderPass.hpp"

namespace Model {
class SingleQuad;
}

class CubemapBuffer;
class FrameBuffer;
class Shader;

class DeferredLightingPass final : public BaseRenderPass {
public:
	explicit DeferredLightingPass(const RenderGraph& graph);

	~DeferredLightingPass() override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
