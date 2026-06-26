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
	~DeferredLightingPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	const Shader* mShader{nullptr};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
