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

	static constexpr float blurKernel[9] = {
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16
	};

	static constexpr float edgeKernel[9] = {
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	};

	static constexpr float sharpenKernel[9] = {
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	};
};
