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

class SSAOPass : public IRenderPass {
public:
	~SSAOPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

	[[nodiscard]] const FrameBuffer* getSSAO() const { return mSSAOBlur.get(); }

	[[nodiscard]] const UniformBuffer* ubo() const { return mSSAOUbo.get(); }

private:
	void ssao(const RenderContext& ctx) const;

	void blur() const;

	std::unique_ptr<Models::SingleQuad> mQuad;
	std::unique_ptr<FrameBuffer> mSSAO;
	std::unique_ptr<FrameBuffer> mSSAOBlur;
	std::unique_ptr<Shader> mSSAOShader;
	std::unique_ptr<Shader> mSSAOBlurShader;
	std::unique_ptr<UniformBuffer> mSSAOUbo;

	std::vector<glm::vec4> mKernel;
	std::vector<float> mNoise;
	uint32_t mNoiseTexture{0};
};
