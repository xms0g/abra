#pragma once
#include "IRenderPass.hpp"

class FrustumCullingPass final : public IRenderPass {
public:
	~FrustumCullingPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};