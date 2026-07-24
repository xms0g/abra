#pragma once
#include "IRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

class Shader;
struct RenderGroup;

class SkyboxPass final : public IRenderPass {
public:
	SkyboxPass();

	~SkyboxPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	RenderQueue<RenderGroup>* mObjects{nullptr};
};
