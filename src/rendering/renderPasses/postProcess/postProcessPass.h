#pragma once
#include <memory>
#include <vector>
#include "IPostEffect.hpp"
#include "../IRenderPass.hpp"

struct RenderContext;

namespace Models {
class Quad;
}

class FrameBuffer;
class Shader;

class PostProcessPass final : public IRenderPass {
public:
	~PostProcessPass() override;

	std::vector<std::shared_ptr<IPostEffect> >& effects();

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<Models::Quad> mQuad;
	std::unique_ptr<FrameBuffer> mRenderTargets[2];
	std::vector<std::shared_ptr<IPostEffect> > mEffects;
};
