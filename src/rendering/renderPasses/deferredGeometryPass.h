#pragma once
#include <memory>
#include <vector>
#include "baseRenderPass.hpp"

struct RenderableObject;
class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public BaseRenderPass {
public:
	explicit DeferredGeometryPass(const RenderContext& ctx);

	~DeferredGeometryPass() override;

	void execute(const RenderContext& ctx, const RenderGraph& graph) override;

private:
	const Shader* mShader{nullptr};
	std::vector<RenderableObject>* mObjects{nullptr};
};
