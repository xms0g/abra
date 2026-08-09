#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class ToneMapping final : public BasePostEffect {
public:
	explicit ToneMapping(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureView render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	float mExposure{1.1f};
};
