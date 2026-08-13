#pragma once
#include "basePostEffect.hpp"
#include "../../descriptorSet.h"

class FrameGraph;
struct RenderContext;
class Shader;
class FrameBuffer;

class Bloom final : public BasePostEffect {
public:
	explicit Bloom(const std::string& name, bool enabled = false);

	~Bloom() override;

	void configure(const FrameGraph& graph) override;

	DescriptorSet* render(GraphicsEncoder& encoder,
	                      DescriptorSet& dscSet,
	                      DescriptorSet& renderTargetDscSet,
	                      FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	DescriptorSet* brightFilterPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle);

	DescriptorSet* blurPass(GraphicsEncoder& encoder, DescriptorSet& dscSet, bool& toggle);

	[[nodiscard]]
	DescriptorSet* combinePass(GraphicsEncoder& encoder,
	                           DescriptorSet& dscSet,
	                           DescriptorSet& blurDscSet,
	                           const bool& toggle);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::array<DescriptorSet, 2> mRenderTargetsDescSets{};
	std::array<GraphicsPipeline, 3> mPipelines;
};
