#pragma once
#include <memory>
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class Kernel final : public BasePostEffect {
public:
	explicit Kernel(const std::string& name, const float* kernel, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	TextureHandle render(uint32_t vao, TextureHandle sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
	const float* mKernel;
};
