#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;

class Sepia final : public BasePostEffect {
public:
	explicit Sepia(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	DescriptorSet* render(GraphicsEncoder& encoder,
	                      DescriptorSet& dscSet,
	                      DescriptorSet& renderTargetDscSet,
	                      FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;
};
