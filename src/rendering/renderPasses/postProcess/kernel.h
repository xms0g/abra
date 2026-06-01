#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class Kernel final : public BasePostEffect {
public:
	explicit Kernel(const std::string& name, const float* kernel, const RenderContext& ctx, bool enabled = false);

	uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
	const float* mKernel;
};
