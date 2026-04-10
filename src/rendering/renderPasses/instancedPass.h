#pragma once
#include "IRenderPass.hpp"
#include "../instanceBufferBuilder.hpp"

class InstancedPass final : public IRenderPass {
public:
	~InstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	InstanceVBO mOpaqueVBO;
	InstanceVBO mBlendVBO;
};