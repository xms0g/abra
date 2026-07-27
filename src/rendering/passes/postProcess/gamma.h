#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class Gamma final : public BasePostEffect {
public:
	explicit Gamma(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(uint32_t vao, TextureHandle sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
