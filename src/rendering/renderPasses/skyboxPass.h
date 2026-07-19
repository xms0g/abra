#pragma once
#include <vector>
#include "IRenderPass.hpp"

class Shader;
struct RenderGroup;

class SkyboxPass final : public IRenderPass {
public:
	explicit SkyboxPass(const RenderContext& ctx);

	~SkyboxPass() override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::vector<RenderGroup>* mObjects{nullptr};
};
