#pragma once
#include "IPass.hpp"
#include "../graphicsPipeline.h"
#include "../buffers/uniformBuffer.h"
#include "../texture/texture.h"

class FrameBuffer;
class Shader;

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

	void createKernel(GraphicsEncoder& encoder);

	void createNoiseTexture();

	struct ResourceIndexes {
		uint32_t gBuffer;
		uint32_t ssao;
		uint32_t blur;
	};

	ResourceIndexes mIndexes{};
	std::array<GraphicsPipeline, 2> mPipelines;
	UniformBuffer mUBO{};
	Texture mNoiseTexture{};
};
