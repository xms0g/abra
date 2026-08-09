#pragma once
#include "basePostEffect.hpp"
#include "../../texture/texture.h"

class FrameGraph;
struct RenderContext;
class Shader;
class FrameBuffer;

class Bloom final : public BasePostEffect {
public:
	explicit Bloom(const std::string& name, bool enabled = false);

	~Bloom() override;

	void configure(const FrameGraph& graph) override;

	TextureView render(GraphicsEncoder& encoder, DescriptorSet& dscSet, FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	TextureView brightFilterPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle);

	TextureView blurPass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, bool& toggle);

	[[nodiscard]]
	TextureView combinePass(GraphicsEncoder& encoder, const DescriptorSet& dscSet, const bool& toggle);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::array<GraphicsPipeline, 3> mPipelines;
};
