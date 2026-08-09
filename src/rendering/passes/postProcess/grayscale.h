#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;

class Grayscale final : public BasePostEffect {
public:
	explicit Grayscale(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureView render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;
};
