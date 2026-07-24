#pragma once
#include "basePostEffect.hpp"

class FrameBuffer;
class Shader;
struct RenderContext;

class Sepia final : public BasePostEffect {
public:
	explicit Sepia(const std::string& name, bool enabled = false);

	void configure(const FrameGraph& graph) override;

	uint32_t render(uint32_t vao, uint32_t sceneTexture, FrameBuffer* renderTarget) const override;

protected:
	void updateFromEventImpl(const GuiPostProcessEvent& event) override;

private:
	const Shader* mShader;
};
