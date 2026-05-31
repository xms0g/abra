#pragma once
#include <memory>
#include "IRenderPass.hpp"

class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IRenderPass {
public:
	~DeferredGeometryPass() override;

	[[nodiscard]]
	const FrameBuffer* gBuffer() const;

	void configure(const RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<FrameBuffer> mGBuffer;
	const Shader* mShader;
};
