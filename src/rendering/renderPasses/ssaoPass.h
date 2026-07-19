#pragma once
#include <cstdint>
#include <memory>
#include "IRenderPass.hpp"
#include "../buffers/uniformBuffer.h"

class FrameBuffer;
class Shader;

namespace Model {
class SingleQuad;
}

class SSAOPass final : public IRenderPass {
public:
	SSAOPass();
	~SSAOPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	void ssao(const RenderGraph& graph) const;

	void blur(const RenderGraph& graph) const;

	std::unique_ptr<Model::SingleQuad> mQuad;
	const Shader* mShader{nullptr};
	const Shader* mBlurShader{nullptr};
	UniformBuffer mUBO{};
};
