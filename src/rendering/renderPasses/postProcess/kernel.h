#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;

class Kernel final : public BasePostEffect {
public:
	explicit Kernel(const std::string& name, const float* kernel, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	std::unique_ptr<Shader> shader;
	const float* mKernel;
};
