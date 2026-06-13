#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class Gamma final : public BasePostEffect {
public:
	explicit Gamma(const std::string& name, const RenderContext& ctx, bool enabled = false);

	uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& renderTargets) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
