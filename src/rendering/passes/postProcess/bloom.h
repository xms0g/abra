#pragma once
#include <memory>
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

	TextureHandle render(uint32_t vao, TextureHandle sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	TextureHandle brightFilterPass(uint32_t vao, TextureHandle sceneTexture, bool& toggle) const;

	TextureHandle blurPass(uint32_t vao, TextureHandle sceneTexture, bool& toggle) const;

	[[nodiscard]]
	TextureHandle combinePass(uint32_t vao, TextureHandle sceneTexture, TextureHandle blurTexture, const bool& toggle) const;

	std::array<FrameBuffer*, 2> mRenderTargets{};

	const Shader* mBrightFilter;
	const Shader* mBlur;
	const Shader* mCombine;
};
