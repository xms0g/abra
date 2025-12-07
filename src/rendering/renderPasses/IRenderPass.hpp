#pragma once

struct RenderContext;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void configure(const RenderContext& ctx) = 0;

	virtual void execute(const RenderContext& ctx) = 0;
};
