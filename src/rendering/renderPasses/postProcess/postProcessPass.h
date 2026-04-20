#pragma once
#include <memory>
#include <vector>
#include "IPostEffect.hpp"
#include "../IRenderPass.hpp"

struct RenderContext;

namespace Models {
class Quad;
}

class Shader;

class PostProcessPass final : public IRenderPass {
public:
	~PostProcessPass() override;

	std::vector<std::unique_ptr<IPostEffect> >& effects();

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	PingPongBuffer mPingPong;
	std::unique_ptr<Models::Quad> mQuad;
	std::vector<std::unique_ptr<IPostEffect> > mEffects;
};
