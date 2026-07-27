#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class ToneMapping final : public BasePostEffect {
public:
	explicit ToneMapping(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(uint32_t vao, TextureHandle sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	float mExposure{1.1f};
	const Shader* mShader;
};
