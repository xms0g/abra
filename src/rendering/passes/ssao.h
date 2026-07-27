#pragma once
#include <cstdint>
#include <memory>
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../graphicsEncoder.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"


class FrameBuffer;
class Shader;

namespace Model {
class SingleQuad;
}

class SSAOPass final : public IPass {
public:
	SSAOPass();

	~SSAOPass() override;

	void configure(const RenderContext& ctx, const FrameGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph) override;

private:
	void ssao();

	void blur();

	void createKernel();

	void createNoiseTexture();

	GraphicsEncoder mEncoder{};
	GraphicsPipeline mSSAOPipeline{};
	GraphicsPipeline mBlurPipeline{};
	UniformBuffer mUBO{};
	Texture mNoiseTexture{};
	std::unique_ptr<Model::SingleQuad> mQuad;
};
