#pragma once
#include "IRenderPass.hpp"
#include "../instanceBufferBuilder.hpp"

class BlendInstancedPass final : public IRenderPass {
public:
	~BlendInstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
	
private:
	InstanceVBO mVBO;
};