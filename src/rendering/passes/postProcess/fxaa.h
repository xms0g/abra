#pragma once
#include <memory>
#include "basePostEffect.hpp"

struct RenderContext;
class FrameBuffer;
class Shader;

class FXAA final : public BasePostEffect {
public:
	explicit FXAA(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(uint32_t vao, TextureHandle sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
