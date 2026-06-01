#pragma once
#include <memory>
#include "basePostEffect.hpp"

struct RenderContext;
class FrameBuffer;
class Shader;

class FXAA final : public BasePostEffect {
public:
	explicit FXAA(const std::string& name, const RenderContext& ctx, bool enabled = false);

	uint32_t render(
		uint32_t vao,
		uint32_t sceneTexture,
		bool& toggle,
		PingPongBuffer& pingPong) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
