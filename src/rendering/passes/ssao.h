#pragma once
#include <cstdint>
#include <memory>
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"

class FrameBuffer;
class Shader;

namespace Model {
class Quad;
}

class SSAOPass final : public IPass {
public:
	SSAOPass();

	~SSAOPass() override;

	void configure(const RenderContext& ctx,
	               const FrameGraph& graph,
	               GraphicsEncoder& encoder,
	               EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const FrameGraph& graph, GraphicsEncoder& encoder) override;

private:
	void ssao(const FrameGraph& graph, GraphicsEncoder& encoder);

	void blur(const FrameGraph& graph, GraphicsEncoder& encoder);

	void createKernel();

	void createNoiseTexture(GraphicsEncoder& encoder);

	std::array<GraphicsPipeline, 2> mPipelines;
	UniformBuffer mUBO{};
	Texture mNoiseTexture{};
	std::unique_ptr<Model::Quad> mQuad;
};
