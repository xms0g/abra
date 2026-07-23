#pragma once
#include "baseRenderPass.hpp"
#include "../renderContext/renderQueue.hpp"

class Shader;
struct RenderGroup;

class SkyboxPass final : public BaseRenderPass {
public:
	explicit SkyboxPass();

	~SkyboxPass() override;

	void configure(const RenderContext& ctx, const RenderGraph& graph, EventBus& eventBus) override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	RenderQueue<RenderGroup>* mObjects{nullptr};
};
