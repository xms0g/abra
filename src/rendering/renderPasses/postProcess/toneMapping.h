#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;

class ToneMapping final : public BasePostEffect {
public:
	explicit ToneMapping(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	float mExposure{1.1f};
	std::unique_ptr<Shader> shader;
};
