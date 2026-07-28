#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;

class Kernel final : public BasePostEffect {
public:
	Kernel(const std::string& name, const float* kernel, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(
		GraphicsEncoder& encoder,
		Model::Quad& quad,
		TextureHandle sceneTexture,
		FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const float* mKernel;
};
