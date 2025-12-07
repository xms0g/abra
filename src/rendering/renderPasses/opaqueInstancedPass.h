#pragma once
#include <cstdint>
#include "IRenderPass.hpp"

class OpaqueInstancedPass final : public IRenderPass {
public:
	~OpaqueInstancedPass() override;

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