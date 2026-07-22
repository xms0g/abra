#pragma once
#include <memory>
#include "basePostEffect.hpp"

struct RenderContext;
class FrameBuffer;
class Shader;

class FXAA final : public BasePostEffect {
public:
	explicit FXAA(const std::string& name, bool enabled = false);

	void configure(const RenderGraph& graph) override;

	uint32_t render(uint32_t vao, uint32_t sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
