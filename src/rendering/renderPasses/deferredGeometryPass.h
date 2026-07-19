#pragma once
#include <memory>
#include <vector>
#include "IRenderPass.hpp"

struct RenderableObject;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IRenderPass {
public:
	explicit DeferredGeometryPass(const RenderContext& ctx);

	~DeferredGeometryPass() override;

	void execute(const RenderContext& ctx, RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::vector<RenderableObject>* mObjects{nullptr};
};
