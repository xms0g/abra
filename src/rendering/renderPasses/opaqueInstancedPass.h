#pragma once
#include "IRenderPass.hpp"
#include "../instanceBufferBuilder.hpp"

class OpaqueInstancedPass final : public IRenderPass {
public:
	~OpaqueInstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	InstanceVBO mVBO;
};