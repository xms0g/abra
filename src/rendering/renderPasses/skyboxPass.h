#pragma once
#include "IRenderPass.hpp"

class SkyboxPass final : public IRenderPass {
public:
	~SkyboxPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};