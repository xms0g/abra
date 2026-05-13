#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;

class Gamma final : public BasePostEffect {
public:
	explicit Gamma(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t sceneTexture,
		uint32_t vao,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	std::unique_ptr<Shader> shader;
};
