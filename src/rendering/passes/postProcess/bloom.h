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

	TextureHandle render(
		GraphicsEncoder& encoder,
		Model::Quad& quad,
		TextureHandle sceneTexture,
		FrameBuffer* renderTarget) override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	TextureHandle brightFilterPass(
		GraphicsEncoder& encoder,
		const Model::Quad& quad,
		TextureHandle sceneTexture,
		bool& toggle);

	TextureHandle blurPass(
		GraphicsEncoder& encoder,
		const Model::Quad& quad,
		TextureHandle sceneTexture,
		bool& toggle);

	[[nodiscard]]
	TextureHandle combinePass(
		GraphicsEncoder& encoder,
		const Model::Quad& quad,
		TextureHandle sceneTexture,
		TextureHandle blurTexture,
		const bool& toggle);

	std::array<FrameBuffer*, 2> mRenderTargets{};
	std::array<GraphicsPipeline, 3> mPipelines;
};
