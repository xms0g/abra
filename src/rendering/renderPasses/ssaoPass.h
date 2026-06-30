#pragma once
#include <cstdint>
#include <memory>
#include "IRenderPass.hpp"

class UniformBuffer;
class FrameBuffer;
class Shader;

namespace Model {
class SingleQuad;
}

class SSAOPass final : public IRenderPass {
public:
	SSAOPass();
	~SSAOPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void ssao(const RenderContext& ctx) const;

	void blur() const;

	std::unique_ptr<Model::SingleQuad> mQuad;
	std::unique_ptr<FrameBuffer> mFBO;
	std::unique_ptr<FrameBuffer> mBlurFBO;
	const Shader* mShader{nullptr};
	const Shader* mBlurShader{nullptr};
	std::unique_ptr<UniformBuffer> mUBO;
};
