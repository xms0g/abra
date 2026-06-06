#pragma once
#include "IRenderPass.hpp"
#include <vector>

namespace math {
	class Frustum;
}

struct RenderGroup;
struct RenderableObject;

class FrustumCullingPass final : public IRenderPass {
public:
	~FrustumCullingPass() override;

	void configure(RenderContext& ctx, EventBus& eventBus) override;

	void execute(const RenderContext& ctx) override;

private:
	void cullScene(
		const RenderContext& ctx,
		const math::Frustum& frustum,
		const std::vector<RenderGroup>& groups,
		std::vector<RenderableObject>& outQueue) const;
};