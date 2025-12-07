#pragma once
#include <cstdint>
#include "IRenderPass.hpp"

class BlendInstancedPass final : public IRenderPass {
public:
	~BlendInstancedPass() override;

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;
	
private:
	void prepareInstanceBuffer(const RenderContext& ctx);
	
	void prepareInstanceData(const RenderContext& ctx) const;
	
	struct {
		uint32_t buffer{};
		int offset{0};
	} vbo;
};