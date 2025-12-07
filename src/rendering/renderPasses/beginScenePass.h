#pragma once
#include "IRenderPass.hpp"

class BeginScenePass final : public IRenderPass {
public:
	~BeginScenePass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
};