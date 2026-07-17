#pragma once
#include <memory>
#include "basePostEffect.hpp"

struct RenderContext;
class FrameBuffer;
class Shader;

class CA final : public BasePostEffect {
public:
	explicit CA(const std::string& name, bool enabled = false);

	uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& renderTargets) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	float mIntensity{};
	const Shader* mShader;
};
