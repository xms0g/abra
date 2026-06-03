#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include "IRenderPass.hpp"

class UniformBuffer;
class FrameBuffer;
class Shader;

namespace Models {
class SingleQuad;
}

class SSAOPass final : public IRenderPass {
public:
	~SSAOPass() override;

	[[nodiscard]]
	const FrameBuffer* blurFBO() const;

	[[nodiscard]]
	const UniformBuffer* ubo() const;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void ssao(const RenderContext& ctx) const;

	void blur() const;

	std::unique_ptr<Models::SingleQuad> mQuad;
	std::unique_ptr<FrameBuffer> mFBO;
	std::unique_ptr<FrameBuffer> mBlurFBO;
	const Shader* mShader;
	const Shader* mBlurShader;
	std::unique_ptr<UniformBuffer> mUBO;
	uint32_t mNoiseTexture{0};
};
