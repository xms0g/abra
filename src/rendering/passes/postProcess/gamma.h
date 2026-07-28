#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;

class Gamma final : public BasePostEffect {
public:
	explicit Gamma(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(
		GraphicsEncoder& encoder,
		Model::Quad& quad,
		TextureHandle sceneTexture,
		FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;
};
