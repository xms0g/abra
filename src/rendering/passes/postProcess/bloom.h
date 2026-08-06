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

	TextureView render(GraphicsEncoder& encoder, TextureView sceneTexture, FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	TextureView brightFilterPass(GraphicsEncoder& encoder, TextureView sceneTexture, bool& toggle);

	TextureView blurPass(GraphicsEncoder& encoder, TextureView sceneTexture, bool& toggle);

	[[nodiscard]]
	TextureView combinePass(GraphicsEncoder& encoder, TextureView sceneTexture, TextureView blurTexture, const bool& toggle);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::array<GraphicsPipeline, 3> mPipelines;
};
