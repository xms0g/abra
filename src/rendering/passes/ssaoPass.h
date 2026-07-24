#pragma once
#include <cstdint>
#include <memory>
#include "IPass.hpp"
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
	void ssao(const FrameGraph& graph) const;

	void blur(const FrameGraph& graph) const;

	std::unique_ptr<Model::SingleQuad> mQuad;
	const Shader* mShader{nullptr};
	const Shader* mBlurShader{nullptr};
	UniformBuffer mUBO{};
	Texture mNoiseTexture{};
};
